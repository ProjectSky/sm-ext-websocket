#include "extension.h"

class YYJsonWrapper {
public:
	YYJsonWrapper();
	~YYJsonWrapper();

	YYJsonWrapper(const YYJsonWrapper&) = delete;
	YYJsonWrapper& operator=(const YYJsonWrapper&) = delete;

public:
	yyjson_mut_doc *m_pDocument_mut;
	yyjson_mut_val *m_pVal_mut;
	Handle_t m_handle;
};