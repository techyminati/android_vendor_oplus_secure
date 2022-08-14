#include "payment.h"
#include "plat_log.h"

#define LOG_TAG "RBS-payment"

void payment_update_last_identify_result(struct LastIdentifyResult* result)
{
	egislog_e("payment_update_last_identify_result need implements");
}

unsigned int payment_update_fingerprint_ids(unsigned int* ids, unsigned int count, char* path)
{
	egislog_e("payment_update_fingerprint_ids need implements");
	return -1;
}

unsigned int payment_get_last_identify_fingerid()
{
	egislog_e("payment_get_last_identify_fingerid need implements");
	return -1;
}

unsigned int payment_get_version() 
{
	egislog_e("payment_get_version need implements");
	return -1;
}

unsigned int payment_setAuthenticatorVersion(unsigned int version, char* path)
{
	egislog_e("payment_setAuthenticatorVersion need implements");
	return -1;
}