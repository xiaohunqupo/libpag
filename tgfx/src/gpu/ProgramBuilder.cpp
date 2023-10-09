/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "ProgramBuilder.h"
#include "gpu/processors/FragmentProcessor.h"

namespace tgfx {
ProgramBuilder::ProgramBuilder(Context* context, const Pipeline* pipeline)
    : context(context), pipeline(pipeline) {
}

bool ProgramBuilder::emitAndInstallProcessors() {
  std::string inputColor;
  std::string inputCoverage;
  emitAndInstallGeoProc(&inputColor, &inputCoverage);
  emitAndInstallFragProcessors(&inputColor, &inputCoverage);
  emitAndInstallXferProc(inputColor, inputCoverage);
  emitFSOutputSwizzle();

  return checkSamplerCounts();
}

void ProgramBuilder::advanceStage() {
  _stageIndex++;
  // Each output to the fragment processor gets its own code section
  fragmentShaderBuilder()->nextStage();
}

void ProgramBuilder::emitAndInstallGeoProc(std::string* outputColor, std::string* outputCoverage) {
  uniformHandler()->addUniform(ShaderFlags::Vertex, SLType::Float4, RTAdjustName);
  advanceStage();
  nameExpression(outputColor, "outputColor");
  nameExpression(outputCoverage, "outputCoverage");
  auto geometryProcessor = pipeline->getGeometryProcessor();
  // Enclose custom code in a block to avoid namespace conflicts
  fragmentShaderBuilder()->codeAppendf("{ // Stage %d %s\n", _stageIndex,
                                       geometryProcessor->name().c_str());
  vertexShaderBuilder()->codeAppendf("// Geometry Processor %s\n",
                                     geometryProcessor->name().c_str());

  GeometryProcessor::FPCoordTransformHandler transformHandler(pipeline, &transformedCoordVars);
  GeometryProcessor::EmitArgs args(vertexShaderBuilder(), fragmentShaderBuilder(), varyingHandler(),
                                   uniformHandler(), getContext()->caps(), *outputColor,
                                   *outputCoverage, &transformHandler);
  geometryProcessor->emitCode(args);
  fragmentShaderBuilder()->codeAppend("}");
}

void ProgramBuilder::emitAndInstallFragProcessors(std::string* color, std::string* coverage) {
  size_t transformedCoordVarsIdx = 0;
  std::string** inOut = &color;
  for (size_t i = 0; i < pipeline->numFragmentProcessors(); ++i) {
    if (i == pipeline->numColorFragmentProcessors()) {
      inOut = &coverage;
    }
    const auto* fp = pipeline->getFragmentProcessor(i);
    auto output = emitAndInstallFragProc(fp, transformedCoordVarsIdx, **inOut);
    FragmentProcessor::Iter iter(fp);
    while (const FragmentProcessor* tempFP = iter.next()) {
      transformedCoordVarsIdx += tempFP->numCoordTransforms();
    }
    **inOut = output;
  }
}

template <typename T>
static const T* GetPointer(const std::vector<T>& vector, size_t atIndex) {
  if (atIndex >= vector.size()) {
    return nullptr;
  }
  return &vector[atIndex];
}

std::string ProgramBuilder::emitAndInstallFragProc(const FragmentProcessor* processor,
                                                   size_t transformedCoordVarsIdx,
                                                   const std::string& input) {
  advanceStage();
  std::string output;
  nameExpression(&output, "output");

  // Enclose custom code in a block to avoid namespace conflicts
  fragmentShaderBuilder()->codeAppendf("{ // Stage %d %s\n", _stageIndex,
                                       processor->name().c_str());

  std::vector<SamplerHandle> texSamplers;
  FragmentProcessor::Iter fpIter(processor);
  int samplerIndex = 0;
  while (const auto* subFP = fpIter.next()) {
    for (size_t i = 0; i < subFP->numTextureSamplers(); ++i) {
      std::string name = "TextureSampler_";
      name += std::to_string(samplerIndex++);
      const auto* sampler = subFP->textureSampler(i);
      texSamplers.emplace_back(emitSampler(sampler, name));
    }
  }
  FragmentProcessor::TransformedCoordVars coords(
      processor, GetPointer(transformedCoordVars, transformedCoordVarsIdx));
  FragmentProcessor::TextureSamplers textureSamplers(processor, GetPointer(texSamplers, 0));
  FragmentProcessor::EmitArgs args(fragmentShaderBuilder(), uniformHandler(), output, input,
                                   &coords, &textureSamplers);

  processor->emitCode(args);

  fragmentShaderBuilder()->codeAppend("}");
  return output;
}

void ProgramBuilder::emitAndInstallXferProc(const std::string& colorIn,
                                            const std::string& coverageIn) {
  advanceStage();

  auto xferProcessor = pipeline->getXferProcessor();
  fragmentShaderBuilder()->codeAppendf("{ // Xfer Processor %s\n", xferProcessor->name().c_str());

  SamplerHandle dstTextureSamplerHandle;
  if (auto dstTexture = pipeline->dstTexture()) {
    dstTextureSamplerHandle = emitSampler(dstTexture->getSampler(), "DstTextureSampler");
  }

  std::string inputColor = !colorIn.empty() ? colorIn : "vec4(1)";
  std::string inputCoverage = !coverageIn.empty() ? coverageIn : "vec4(1)";
  XferProcessor::EmitArgs args(fragmentShaderBuilder(), uniformHandler(), inputColor, inputCoverage,
                               fragmentShaderBuilder()->colorOutputName(), dstTextureSamplerHandle);
  xferProcessor->emitCode(args);
  fragmentShaderBuilder()->codeAppend("}");
}

SamplerHandle ProgramBuilder::emitSampler(const TextureSampler* sampler, const std::string& name) {
  ++numFragmentSamplers;
  return uniformHandler()->addSampler(sampler, name);
}

void ProgramBuilder::emitFSOutputSwizzle() {
  // Swizzle the fragment shader outputs if necessary.
  const auto& swizzle = *pipeline->outputSwizzle();
  if (swizzle == Swizzle::RGBA()) {
    return;
  }
  auto* fragBuilder = fragmentShaderBuilder();
  const auto& output = fragBuilder->colorOutputName();
  fragBuilder->codeAppendf("%s = %s.%s;", output.c_str(), output.c_str(), swizzle.c_str());
}

std::string ProgramBuilder::nameVariable(char prefix, const std::string& name, bool mangle) const {
  std::string out;
  if ('\0' != prefix) {
    out += prefix;
  }
  out += name;
  if (mangle && _stageIndex >= 0) {
    if (out.rfind('_') == out.length() - 1) {
      // Names containing "__" are reserved.
      out += "x";
    }
    out += "_Stage";
    out += std::to_string(_stageIndex);
  }
  return out;
}

void ProgramBuilder::nameExpression(std::string* output, const std::string& baseName) {
  // create var to hold stage result. If we already have a valid output name, just use that
  // otherwise create a new mangled one. This name is only valid if we are reordering stages
  // and have to tell stage exactly where to put its output.
  std::string outName;
  if (!output->empty()) {
    outName = *output;
  } else {
    outName = nameVariable('\0', baseName);
  }
  fragmentShaderBuilder()->codeAppendf("vec4 %s;", outName.c_str());
  *output = outName;
}

std::string ProgramBuilder::getUniformDeclarations(ShaderFlags visibility) const {
  return uniformHandler()->getUniformDeclarations(visibility);
}

void ProgramBuilder::finalizeShaders() {
  varyingHandler()->finalize();
  vertexShaderBuilder()->finalize(ShaderFlags::Vertex);
  fragmentShaderBuilder()->finalize(ShaderFlags::Fragment);
}
}  // namespace tgfx
