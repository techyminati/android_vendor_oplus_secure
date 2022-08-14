#ifndef __FACTORY_TEST_CA_H__
#define __FACTORY_TEST_CA_H__

#include "anc_error.h"
#include "anc_type.h"
#include "extension_command.h"


ANC_RETURN_TYPE ExtensionFactoryTest(AncFactoryTestCommand *p_factory_test_command,
                                    AncFactoryTestCommandRespond **p_factory_test_command_respond);

#endif

