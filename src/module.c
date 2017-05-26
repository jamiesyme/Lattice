#include <stddef.h>

#include "app-config.h"
#include "module.h"


void initModule(Module* module, AppConfig* appConfig)
{
  module->appConfig = appConfig;
  module->type = MT_UNKNOWN;
  module->rect = (Rect){0.0f, 0.0f, 0.0f, 0.0f};
  module->depth = 0.0f;
  module->renderFunc = NULL;
  module->freeFunc = NULL;
  module->extraData = NULL;
}

void setModuleDrawSize(Module* module, Dimensions drawSize)
{
  float borderSize = module->appConfig->moduleBorderSize;
  Dimensions paddingSize = module->appConfig->modulePaddingSize;
  module->rect.width = (borderSize * 1 +
                        paddingSize.width * 2 +
                        drawSize.width);
  module->rect.height = (borderSize * 2 +
                         paddingSize.height * 2 +
                         drawSize.height);
}
