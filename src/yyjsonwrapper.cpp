#include "extension.h"

YYJsonWrapper::YYJsonWrapper() : m_pDocument_mut(NULL), m_pVal_mut(NULL), m_handle(BAD_HANDLE) {}

YYJsonWrapper::~YYJsonWrapper() {
	if (this->m_pDocument_mut) {
		yyjson_mut_doc_free(this->m_pDocument_mut);
		this->m_pDocument_mut = NULL;
	}
}