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

#pragma once

#include "task/PAGTask.h"

namespace pag {

class PAGExportPNGTask : public PAGPlayTask {
  Q_OBJECT
 public:
  explicit PAGExportPNGTask(std::shared_ptr<PAGFile>& pagFile, const QString& filePath,
                            int exportFrame = -1);

 protected:
  void onFrameFlush(double progress) override;
  bool isNeedRenderCurrentFrame() override;

 private:
  void exportCurrentFrameAsPNG(const QString& outPath);

 private:
  int exportFrame = -1;
};

}  // namespace pag