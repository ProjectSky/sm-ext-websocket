#include "extension.h"

static cell_t pawn_json_parse(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool is_file = params[2];

	yyjson_read_err readError;
	yyjson_doc *idoc;

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();

	if (is_file)
	{
		char realpath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", string);
		idoc = yyjson_read_file(realpath, params[3], NULL, &readError);

		if (readError.code) {
			pContext->ReportError("read error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
			return BAD_HANDLE;
		}

		pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
		pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
	}
	else
	{
		idoc = yyjson_read_opts(string, strlen(string), params[3], NULL, &readError);

		if (readError.code) {
			pContext->ReportError("read error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
			return BAD_HANDLE;
		}

		pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
	}

	yyjson_doc_free(idoc);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create parse handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_equals(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle1 = g_WebsocketExt.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper *handle2 = g_WebsocketExt.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	return yyjson_mut_equals(handle1->m_pVal_mut, handle2->m_pVal_mut);
}

static cell_t pawn_json_deep_copy(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle1 = g_WebsocketExt.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper *handle2 = g_WebsocketExt.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create deep_copy handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_get_type_desc(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_type_desc(handle->m_pVal_mut), NULL);

	return 1;
}

static cell_t pawn_json_object_from_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc *idoc = yyjson_read_opts(string, strlen(string), params[2], NULL, &readError);

	if (readError.code) {
		pContext->ReportError("read error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
	yyjson_doc_free(idoc);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create object_from_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_object_from_file(IPluginContext *pContext, const cell_t *params)
{
	char *path;
	pContext->LocalToString(params[1], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc *idoc = yyjson_read_file(realpath, params[2], NULL, &readError);

	if (readError.code) {
		pContext->ReportError("read error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
	yyjson_doc_free(idoc);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create object_from_file handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_array_from_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);
	
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc *idoc = yyjson_read_opts(string, strlen(string), params[2], NULL, &readError);

	if (readError.code) {
		pContext->ReportError("read error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
	yyjson_doc_free(idoc);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create array_from_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_array_from_file(IPluginContext *pContext, const cell_t *params)
{
	char *path;
	pContext->LocalToString(params[1], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	
	yyjson_read_err readError;
	yyjson_doc *idoc = yyjson_read_file(realpath, params[2], NULL, &readError);

	if (readError.code) {
		pContext->ReportError("read error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
	yyjson_doc_free(idoc);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create array_from_file handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_get_type(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_get_type(handle->m_pVal_mut);
}

static cell_t pawn_json_get_subtype(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_get_subtype(handle->m_pVal_mut);
}

static cell_t pawn_json_is_array(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_is_arr(handle->m_pVal_mut);
}

static cell_t pawn_json_is_object(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_is_obj(handle->m_pVal_mut);
}

static cell_t pawn_json_init_object(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj(pYYJsonWrapper->m_pDocument_mut);

	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut, pYYJsonWrapper->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create init_object handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_create_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_bool(pYYJsonWrapper->m_pDocument_mut, params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create create_bool handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_create_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_float(pYYJsonWrapper->m_pDocument_mut, sp_ctof(params[1]));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create create_float handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_create_int(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_int(pYYJsonWrapper->m_pDocument_mut, params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create create_int handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_create_intger64(IPluginContext *pContext, const cell_t *params)
{
	char *val;
	pContext->LocalToString(params[1], &val);

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_uint(pYYJsonWrapper->m_pDocument_mut, strtoll(val, NULL, 10));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create create_intger64 handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_create_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_str(pYYJsonWrapper->m_pDocument_mut, string);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create create_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_get_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_bool(handle->m_pVal_mut)) {
		pContext->ReportError("JSON value is not a bool!");
		return 0;
	}

	return yyjson_mut_get_bool(handle->m_pVal_mut);
}

static cell_t pawn_json_get_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_real(handle->m_pVal_mut)) {
		pContext->ReportError("JSON value is not a float!");
		return 0;
	}

	return sp_ftoc(yyjson_mut_get_real(handle->m_pVal_mut));
}

static cell_t pawn_json_get_int(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_int(handle->m_pVal_mut)) {
		pContext->ReportError("JSON value is not a integer!");
		return 0;
	}

	return yyjson_mut_get_int(handle->m_pVal_mut);
}

static cell_t pawn_json_get_intger64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_uint(handle->m_pVal_mut)) {
		pContext->ReportError("JSON value is not a integer64!");
		return 0;
	}

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(handle->m_pVal_mut));
	pContext->StringToLocalUTF8(params[2], params[3], result, NULL);

	return 1;
}

static cell_t pawn_json_get_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_str(handle->m_pVal_mut)) {
		pContext->ReportError("JSON value is not a str!");
		return 0;
	}

	pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_str(handle->m_pVal_mut), NULL);

	return 1;
}

static cell_t pawn_json_create_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_null(pYYJsonWrapper->m_pDocument_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create create_null handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_init_array(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(NULL);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr(pYYJsonWrapper->m_pDocument_mut);
	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut, pYYJsonWrapper->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create init_array handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_array_get_count(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_size(handle->m_pVal_mut);
}

static cell_t pawn_json_array_get_value(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create array_get_value handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_array_get_first(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_get_first(handle->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create array_get_first handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_array_get_last(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_get_last(handle->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create array_get_last handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_array_get_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val *arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	return yyjson_mut_get_bool(arr);
}

static cell_t pawn_json_array_get_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val *arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	return sp_ftoc(yyjson_mut_get_real(arr));
}

static cell_t pawn_json_array_get_integer(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val *arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	return yyjson_mut_get_int(arr);
}

static cell_t pawn_json_array_get_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val *arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(arr));
	pContext->StringToLocalUTF8(params[3], params[4], result, NULL);

	return 1;
}

static cell_t pawn_json_array_get_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val *arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;
	
	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(arr), NULL);

	return 1;
}

static cell_t pawn_json_array_is_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val *arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	return yyjson_mut_is_null(arr);
}

static cell_t pawn_json_array_replace_value(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], handle->m_pVal_mut) != NULL;
}

static cell_t pawn_json_array_replace_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_bool(handle->m_pDocument_mut, params[3])) != NULL;
}

static cell_t pawn_json_array_replace_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_float(handle->m_pDocument_mut, sp_ftoc(params[3]))) != NULL;
}

static cell_t pawn_json_array_replace_integer(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_int(handle->m_pDocument_mut, params[3])) != NULL;
}

static cell_t pawn_json_array_replace_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *val;
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_uint(handle->m_pDocument_mut, strtoll(val, NULL, 10))) != NULL;
}

static cell_t pawn_json_array_replace_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_null(handle->m_pDocument_mut)) != NULL;
}

static cell_t pawn_json_array_replace_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *val;
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_str(handle->m_pDocument_mut, val)) != NULL;
}

static cell_t pawn_json_array_append_value(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle1 = g_WebsocketExt.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper *handle2 = g_WebsocketExt.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle1->m_pVal_mut, handle2->m_pVal_mut);
}

static cell_t pawn_json_array_append_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_bool(handle->m_pDocument_mut, params[2]));
}

static cell_t pawn_json_array_append_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[2])));
}

static cell_t pawn_json_array_append_integer(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_int(handle->m_pDocument_mut, params[2]));
}

static cell_t pawn_json_array_append_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *val;
	pContext->LocalToString(params[2], &val);

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_uint(handle->m_pDocument_mut, strtoll(val, NULL, 10)));
}

static cell_t pawn_json_array_append_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_null(handle->m_pDocument_mut));
}

static cell_t pawn_json_array_append_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *str;
	pContext->LocalToString(params[2], &str);

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, str));
}

static cell_t pawn_json_array_remove(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove(handle->m_pVal_mut, params[2]) != NULL;
}

static cell_t pawn_json_array_remove_first(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove_first(handle->m_pVal_mut) != NULL;
}

static cell_t pawn_json_array_remove_last(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove_last(handle->m_pVal_mut) != NULL;
}

static cell_t pawn_json_array_remove_range(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove_range(handle->m_pVal_mut, params[2], params[3]);
}

static cell_t pawn_json_array_clear(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_clear(handle->m_pVal_mut);
}

static cell_t pawn_json_to_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *json_str = yyjson_mut_val_write(handle->m_pVal_mut, params[4], NULL);

	if (json_str) {
		pContext->StringToLocalUTF8(params[2], params[3], json_str, NULL);

		free(json_str);
		return 1;
	}

	return 0;
}

static cell_t pawn_json_to_file(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);

	yyjson_write_err writeError;

	bool is_success = yyjson_mut_write_file(realpath, handle->m_pDocument_mut, params[3], NULL, &writeError);

	if (writeError.code) {
		pContext->ReportError("write error (%u): %s", writeError.code, writeError.msg);
		return BAD_HANDLE;
	}

	return is_success;
}

static cell_t pawn_json_object_get_count(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_obj_size(handle->m_pVal_mut);
}

static cell_t pawn_json_object_get_key_name_by_index(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_obj_iter iter;
	yyjson_mut_obj_iter_init(handle->m_pVal_mut, &iter);

	for (size_t i = 0; i < params[2]; i++) {
		yyjson_mut_obj_iter_next(&iter);
	}

	yyjson_mut_val *key = yyjson_mut_obj_iter_next(&iter);
	if (!key) {
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(key), NULL);

	return 1;
}

static cell_t pawn_json_object_get_value_at(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_obj_iter iter;
	yyjson_mut_obj_iter_init(handle->m_pVal_mut, &iter);

	for (size_t i = 0; i < params[2]; i++) {
		yyjson_mut_obj_iter_next(&iter);
	}

	yyjson_mut_val *key = yyjson_mut_obj_iter_next(&iter);
	if (!key) {
		return 0;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();

	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj_iter_get_val(key);
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create object_get_value_at handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_object_get_value(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create array_get_value handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_object_get_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return yyjson_mut_get_bool(obj);
}

static cell_t pawn_json_object_get_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return sp_ftoc(yyjson_mut_get_real(obj));
}

static cell_t pawn_json_object_get_integer(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return yyjson_mut_get_int(obj);
}

static cell_t pawn_json_object_get_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(obj));
	pContext->StringToLocalUTF8(params[3], params[4], result, NULL);

	return 1;
}

static cell_t pawn_json_object_get_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;
	
	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;
	
	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(obj), NULL);

	return 1;
}

static cell_t pawn_json_object_clear(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return yyjson_mut_obj_clear(obj);
}

static cell_t pawn_json_object_is_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return yyjson_mut_is_null(obj);
}

static cell_t pawn_json_object_has_key(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_obj_iter iter = yyjson_mut_obj_iter_with(handle->m_pVal_mut);

	bool ptr_use = params[3];
	
	yyjson_mut_val *val = ptr_use ? yyjson_mut_doc_ptr_get(handle->m_pDocument_mut, key) : yyjson_mut_obj_iter_get(&iter, key);

	return val != NULL;
}

static cell_t pawn_json_object_rename_key(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key, *new_key;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &new_key);

	return yyjson_mut_obj_rename_key(handle->m_pDocument_mut, handle->m_pVal_mut, key, new_key);
}

static cell_t pawn_json_object_set_value(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle1 = g_WebsocketExt.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper *handle2 = g_WebsocketExt.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val *val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	return yyjson_mut_obj_put(handle1->m_pVal_mut, yyjson_mut_str(handle1->m_pDocument_mut, key), val_copy);
}

static cell_t pawn_json_object_set_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_bool(handle->m_pDocument_mut,  params[3]));
}

static cell_t pawn_json_object_set_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;
	
	char *key;
	pContext->LocalToString(params[2], &key);
	
	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3])));
}

static cell_t pawn_json_object_set_integer(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_int(handle->m_pDocument_mut, params[3]));
}

static cell_t pawn_json_object_set_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key, *val;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_uint(handle->m_pDocument_mut, strtoll(val, NULL, 10)));
}

static cell_t pawn_json_object_set_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_null(handle->m_pDocument_mut));
}

static cell_t pawn_json_object_set_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_str(handle->m_pDocument_mut, value));
}

static cell_t pawn_json_object_remove(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_remove_key(handle->m_pVal_mut, key) != NULL;
}

static cell_t pawn_json_pointer_get(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper *pYYJsonWrapper = new YYJsonWrapper();

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, NULL, &err);
	
	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Could not create json_pointer_get handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pYYJsonWrapper->m_handle;
}

static cell_t pawn_json_pointer_get_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val *ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_bool(ptr)) {
		pContext->ReportError("JSON value at path '%s' is not a boolean", path);
		return 0;
	}

	return yyjson_mut_get_bool(ptr);
}

static cell_t pawn_json_pointer_get_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val *ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	return sp_ftoc(yyjson_mut_get_real(ptr));
}

static cell_t pawn_json_pointer_get_int(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val *ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	return yyjson_mut_get_int(ptr);
}

static cell_t pawn_json_pointer_get_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val *ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(ptr));
	pContext->StringToLocalUTF8(params[3], params[4], result, NULL);

	return 1;
}

static cell_t pawn_json_pointer_get_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val *ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_str(ptr)) {
		pContext->ReportError("JSON value at path '%s' is not a string", path);
		return 0;
	}
	
	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(ptr), NULL);

	return 1;
}

static cell_t pawn_json_pointer_get_is_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val *ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), NULL, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("JSON pointer get error (%u): %s at position: %d", ptrGetError.code, ptrGetError.msg, ptrGetError.pos);
		return BAD_HANDLE;
	}

	return yyjson_mut_is_null(ptr);
}

static cell_t pawn_json_pointer_set(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle1 = g_WebsocketExt.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper *handle2 = g_WebsocketExt.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_mut_val *val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle1->m_pDocument_mut, path, strlen(path), val_copy, true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_set_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_bool(handle->m_pDocument_mut, params[3]), true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_set_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3])), true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_set_int(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, params[3]), true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_set_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path, *val;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &val);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, strtoll(val, NULL, 10)), true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_set_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path, *str;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &str);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_str(handle->m_pDocument_mut, str), true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_set_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_null(handle->m_pDocument_mut), true, NULL, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("JSON pointer set error (%u): %s at position: %d", ptrSetError.code, ptrSetError.msg, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle1 = g_WebsocketExt.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper *handle2 = g_WebsocketExt.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_mut_val *val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle1->m_pDocument_mut, path, strlen(path), val_copy, true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_bool(handle->m_pDocument_mut, params[3]), true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add_float(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3])), true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add_int(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, params[3]), true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add_integer64(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path, *val;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &val);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, strtoll(val, NULL, 10)), true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add_string(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path, *str;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &str);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_str(handle->m_pDocument_mut, str), true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_add_null(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_null(handle->m_pDocument_mut), true, NULL, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("JSON pointer add error (%u): %s at position: %d", ptrAddError.code, ptrAddError.msg, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t pawn_json_pointer_remove(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_WebsocketExt.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char *path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrRemoveError;
	bool success = yyjson_mut_ptr_removex(handle->m_pVal_mut, path, strlen(path), NULL, &ptrRemoveError) != NULL;

	if (ptrRemoveError.code) {
		pContext->ReportError("JSON pointer remove error (%u): %s at position: %d", ptrRemoveError.code, ptrRemoveError.msg, ptrRemoveError.pos);
		return BAD_HANDLE;
	}

	return success;
}

const sp_nativeinfo_t json_natives[] =
{
	// JSONObject
	{"YYJSONObject.YYJSONObject", pawn_json_init_object},
	{"YYJSONObject.Size.get", pawn_json_object_get_count},
	{"YYJSONObject.Get", pawn_json_object_get_value},
	{"YYJSONObject.GetBool", pawn_json_object_get_bool},
	{"YYJSONObject.GetFloat", pawn_json_object_get_float},
	{"YYJSONObject.GetInt", pawn_json_object_get_integer},
	{"YYJSONObject.GetInt64", pawn_json_object_get_integer64},
	{"YYJSONObject.GetString", pawn_json_object_get_string},
	{"YYJSONObject.Clear", pawn_json_object_clear},
	{"YYJSONObject.IsNull", pawn_json_object_is_null},
	{"YYJSONObject.GetKeyName", pawn_json_object_get_key_name_by_index},
	{"YYJSONObject.GetValueAt", pawn_json_object_get_value_at},
	{"YYJSONObject.HasKey", pawn_json_object_has_key},
	{"YYJSONObject.RenameKey", pawn_json_object_rename_key},
	{"YYJSONObject.Set", pawn_json_object_set_value},
	{"YYJSONObject.SetBool", pawn_json_object_set_bool},
	{"YYJSONObject.SetFloat", pawn_json_object_set_float},
	{"YYJSONObject.SetInt", pawn_json_object_set_integer},
	{"YYJSONObject.SetInt64", pawn_json_object_set_integer64},
	{"YYJSONObject.SetNull", pawn_json_object_set_null},
	{"YYJSONObject.SetString", pawn_json_object_set_string},
	{"YYJSONObject.Remove", pawn_json_object_remove},
	{"YYJSONObject.Clear", pawn_json_object_clear},
	{"YYJSONObject.FromString", pawn_json_object_from_string},
	{"YYJSONObject.FromFile", pawn_json_object_from_file},

	// JSONArray
	{"YYJSONArray.YYJSONArray", pawn_json_init_array},
	{"YYJSONArray.Length.get", pawn_json_array_get_count},
	{"YYJSONArray.Get", pawn_json_array_get_value},
	{"YYJSONArray.First.get", pawn_json_array_get_first},
	{"YYJSONArray.Last.get", pawn_json_array_get_last},
	{"YYJSONArray.GetBool", pawn_json_array_get_bool},
	{"YYJSONArray.GetFloat", pawn_json_array_get_float},
	{"YYJSONArray.GetInt", pawn_json_array_get_integer},
	{"YYJSONArray.GetInt64", pawn_json_array_get_integer64},
	{"YYJSONArray.GetString", pawn_json_array_get_string},
	{"YYJSONArray.IsNull", pawn_json_array_is_null},
	{"YYJSONArray.Set", pawn_json_array_replace_value},
	{"YYJSONArray.SetBool", pawn_json_array_replace_bool},
	{"YYJSONArray.SetFloat", pawn_json_array_replace_float},
	{"YYJSONArray.SetInt", pawn_json_array_replace_integer},
	{"YYJSONArray.SetInt64", pawn_json_array_replace_integer64},
	{"YYJSONArray.SetNull", pawn_json_array_replace_null},
	{"YYJSONArray.SetString", pawn_json_array_replace_string},
	{"YYJSONArray.Push", pawn_json_array_append_value},
	{"YYJSONArray.PushBool", pawn_json_array_append_bool},
	{"YYJSONArray.PushFloat", pawn_json_array_append_float},
	{"YYJSONArray.PushInt", pawn_json_array_append_integer},
	{"YYJSONArray.PushInt64", pawn_json_array_append_integer64},
	{"YYJSONArray.PushNull", pawn_json_array_append_null},
	{"YYJSONArray.PushString", pawn_json_array_append_string},
	{"YYJSONArray.Remove", pawn_json_array_remove},
	{"YYJSONArray.RemoveFirst", pawn_json_array_remove_first},
	{"YYJSONArray.RemoveLast", pawn_json_array_remove_last},
	{"YYJSONArray.RemoveRange", pawn_json_array_remove_range},
	{"YYJSONArray.Clear", pawn_json_array_clear},
	{"YYJSONArray.FromString", pawn_json_array_from_string},
	{"YYJSONArray.FromFile", pawn_json_array_from_file},

	// JSON
	{"YYJSON.ToString", pawn_json_to_string},
	{"YYJSON.ToFile", pawn_json_to_file},
	{"YYJSON.Parse", pawn_json_parse},
	{"YYJSON.Equals", pawn_json_equals},
	{"YYJSON.DeepCopy", pawn_json_deep_copy},
	{"YYJSON.GetTypeDesc", pawn_json_get_type_desc},
	{"YYJSON.Type.get", pawn_json_get_type},
	{"YYJSON.SubType.get", pawn_json_get_subtype},
	{"YYJSON.IsArray.get", pawn_json_is_array},
	{"YYJSON.IsObject.get", pawn_json_is_object},

	// JSON CREATE & GET
	{"YYJSON.CreateBool", pawn_json_create_bool},
	{"YYJSON.CreateFloat", pawn_json_create_float},
	{"YYJSON.CreateInt", pawn_json_create_int},
	{"YYJSON.CreateInt64", pawn_json_create_intger64},
	{"YYJSON.CreateNull", pawn_json_create_null},
	{"YYJSON.CreateString", pawn_json_create_string},
	{"YYJSON.GetBool", pawn_json_get_bool},
	{"YYJSON.GetFloat", pawn_json_get_float},
	{"YYJSON.GetInt", pawn_json_get_int},
	{"YYJSON.GetInt64", pawn_json_get_intger64},
	{"YYJSON.GetString", pawn_json_get_string},

	// JSON POINTER
	{"YYJSON.PtrGet", pawn_json_pointer_get},
	{"YYJSON.PtrGetBool", pawn_json_pointer_get_bool},
	{"YYJSON.PtrGetFloat", pawn_json_pointer_get_float},
	{"YYJSON.PtrGetInt", pawn_json_pointer_get_int},
	{"YYJSON.PtrGetInt64", pawn_json_pointer_get_integer64},
	{"YYJSON.PtrGetString", pawn_json_pointer_get_string},
	{"YYJSON.PtrGetIsNull", pawn_json_pointer_get_is_null},
	{"YYJSON.PtrSet", pawn_json_pointer_set},
	{"YYJSON.PtrSetBool", pawn_json_pointer_set_bool},
	{"YYJSON.PtrSetFloat", pawn_json_pointer_set_float},
	{"YYJSON.PtrSetInt", pawn_json_pointer_set_int},
	{"YYJSON.PtrSetInt64", pawn_json_pointer_set_integer64},
	{"YYJSON.PtrSetString", pawn_json_pointer_set_string},
	{"YYJSON.PtrSetNull", pawn_json_pointer_set_null},
	{"YYJSON.PtrAdd", pawn_json_pointer_add},
	{"YYJSON.PtrAddBool", pawn_json_pointer_add_bool},
	{"YYJSON.PtrAddFloat", pawn_json_pointer_add_float},
	{"YYJSON.PtrAddInt", pawn_json_pointer_add_int},
	{"YYJSON.PtrAddInt64", pawn_json_pointer_add_integer64},
	{"YYJSON.PtrAddString", pawn_json_pointer_add_string},
	{"YYJSON.PtrAddNull", pawn_json_pointer_add_null},
	{"YYJSON.PtrRemove", pawn_json_pointer_remove},
	{NULL, NULL}
};