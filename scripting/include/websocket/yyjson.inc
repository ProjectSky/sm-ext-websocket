enum YYJSON_TYPE
{
  YYJSON_TYPE_NONE = 0,
  YYJSON_TYPE_RAW  = 1,
  YYJSON_TYPE_NULL = 2,
  YYJSON_TYPE_BOOL = 3,
  YYJSON_TYPE_NUM  = 4,
  YYJSON_TYPE_STR  = 5,
  YYJSON_TYPE_ARR  = 6,
  YYJSON_TYPE_OBJ  = 7
}

enum YYJSON_SUBTYPE
{
  YYJSON_SUBTYPE_NONE  = 0 << 3,
  YYJSON_SUBTYPE_FALSE = 0 << 3,
  YYJSON_SUBTYPE_TRUE  = 1 << 3,
  YYJSON_SUBTYPE_UINT  = 0 << 3,
  YYJSON_SUBTYPE_SINT  = 1 << 3,
  YYJSON_SUBTYPE_REAL  = 2 << 3,
  YYJSON_SUBTYPE_NOESC = 1 << 3,
}

enum YYJSON_READ_FLAG
{
  YYJSON_READ_NOFLAG                  = 0 << 0,
  YYJSON_READ_INSITU                  = 1 << 0,
  YYJSON_READ_STOP_WHEN_DONE          = 1 << 1,
  YYJSON_READ_ALLOW_TRAILING_COMMAS   = 1 << 2,
  YYJSON_READ_ALLOW_COMMENTS          = 1 << 3,
  YYJSON_READ_ALLOW_INF_AND_NAN       = 1 << 4,
  YYJSON_READ_NUMBER_AS_RAW           = 1 << 5,
  YYJSON_READ_ALLOW_INVALID_UNICODE   = 1 << 6,
  YYJSON_READ_BIGNUM_AS_RAW           = 1 << 7,
}

enum YYJSON_WRITE_FLAG
{
  YYJSON_WRITE_NOFLAG                 = 0 << 0,
  YYJSON_WRITE_PRETTY                 = 1 << 0,
  YYJSON_WRITE_ESCAPE_UNICODE         = 1 << 1,
  YYJSON_WRITE_ESCAPE_SLASHES         = 1 << 2,
  YYJSON_WRITE_ALLOW_INF_AND_NAN      = 1 << 3,
  YYJSON_WRITE_INF_AND_NAN_AS_NULL    = 1 << 4,
  YYJSON_WRITE_ALLOW_INVALID_UNICODE  = 1 << 5,
  YYJSON_WRITE_PRETTY_TWO_SPACES      = 1 << 6,
  YYJSON_WRITE_NEWLINE_AT_END         = 1 << 7,
}

/**
* Write floating-point number using fixed-point notation
* - This is similar to ECMAScript `Number.prototype.toFixed(prec)`, but with trailing zeros removed. The `prec` ranges from 1 to 15
* - This will produce shorter output but may lose some precision
*/
stock YYJSON_WRITE_FLAG YYJSON_REAL_PRECISION(int n)
{
	return view_as<YYJSON_WRITE_FLAG>((n) << 28);
}

methodmap YYJSON < Handle
{
  /**
  * Write a document to JSON file with options
  * 
  * @note                    On 32-bit operating system, files larger than 2GB may fail to write
  *
  * @param file              The JSON file's path. If this path is NULL or invalid, the function will fail and return false. If this file is not empty, the content will be discarded
  * @param flag              The JSON write options
  * 
  * @return                  True on success, false on failure
  */
  public native bool ToFile(const char[] file, YYJSON_WRITE_FLAG flag = YYJSON_WRITE_NOFLAG);

  /**
  * Write a value to JSON string
  *
  * @param buffer            String buffer to write to
  * @param maxlength         Maximum length of the string buffer
  * @param flag              The JSON write options
  * 
  * @return                  True on success, false on failure
  */
  public native bool ToString(char[] buffer, int maxlength, YYJSON_WRITE_FLAG flag = YYJSON_WRITE_NOFLAG);

  /**
  * Parses JSON string or a file that contains JSON
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param string            String or file to parse
  * @param is_file           True to treat string param as file, false otherwise
  * @param flag              The JSON read options
  *
  * @return                  JSON handle, false on failure
  */
  public static native any Parse(const char[] string, bool is_file = false, YYJSON_READ_FLAG flag = YYJSON_READ_NOFLAG);

  /**
  * Copies and returns a new mutable value from input, returns NULL on error. This makes a deep-copy on the mutable value. The memory was managed by mutable document
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  * @note                    This function is recursive and may cause a stack overflow if the object level is too deep
  *
  * @param value1            JSON handle1
  * @param value2            JSON handle2
  *
  * @return                  JSON handle, false on failure
  */
  public static native any DeepCopy(const YYJSON value1, const YYJSON value2);

  /**
  * Returns the JSON value's type description
  *
	* @param value             JSON handle
  * @param buffer            String buffer to write to
  * @param maxlength         Maximum length of the string buffer
  * 
  * @return                  The return value should be one of these strings: "raw", "null", "string", 
	*                          "array", "object", "true", "false", "uint", "sint", "real", "unknown".
  */
	public static native void GetTypeDesc(const YYJSON value, char[] buffer, int maxlength);

  /**
  * Returns whether two JSON values are equal (deep compare)
  *
  * @note                    This function is recursive and may cause a stack overflow if the object level is too deep
  * @note                    the result may be inaccurate if object has duplicate keys
  *
  * @param value1            JSON handle
  * @param value2            JSON handle
  *
  * @return                  True if they are the same, false otherwise
  */
  public static native bool Equals(const YYJSON value1, const YYJSON value2);

  /**
  * Creates and returns a boolean value
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param value             The boolean value to be set
  *
  * @return                  JSON handle, NULL on error
  */
  public static native YYJSON CreateBool(bool value);

  /**
  * Creates and returns a float value
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param value             The float value to be set
  *
  * @return                  JSON handle, NULL on error
  */
  public static native YYJSON CreateFloat(float value);

  /**
  * Creates and returns a int value
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param value             The int value to be set
  *
  * @return                  JSON handle, NULL on error
  */
  public static native YYJSON CreateInt(int value);
  
  /**
  * Creates and returns a intger64 value
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param value             The intger64 value to be set
  *
  * @return                  JSON handle, NULL on error
  */
  public static native YYJSON CreateInt64(const char[] value);

  /**
  * Creates and returns a string value
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param value             The string value to be set
  *
  * @return                  JSON handle, NULL on error
  */
  public static native YYJSON CreateString(const char[] value);

  /**
  * Creates and returns a null value
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @return                  JSON handle, NULL on error
  */
  public static native YYJSON CreateNull();

  /**
  * Get boolean value by a JSON Handle
  *
  * @param value             JSON handle
  *
  * @return                  Boolean value
  */
  public static native bool GetBool(const YYJSON value);

  /**
  * Get float value by a JSON Handle
  *
  * @param value             JSON handle
  *
  * @return                  float value
  */
  public static native float GetFloat(const YYJSON value);

  /**
  * Get int value by a JSON Handle
  *
  * @param value             JSON handle
  *
  * @return                  int value
  */
  public static native int GetInt(const YYJSON value);

  /**
  * Get intger64 value by a JSON Handle
  *
  * @param value             JSON handle
  * @param buffer            Buffer to copy to
  * @param maxlength         Maximum size of the buffer
  * 
  * @return                  True on success, false on failure
  */
  public static native bool GetInt64(const YYJSON value, char[] buffer, int maxlength);

  /**
  * Get string value by a JSON Handle
  *
  * @param value             JSON handle
  * @param buffer            Buffer to copy to
  * @param maxlength         Maximum size of the buffer
  * 
  * @return                  True on success, false on failure
  */
  public static native bool GetString(const YYJSON value, char[] buffer, int maxlength);

  /**
  * Get value by a JSON Pointer
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  * @return                  The value referenced by the JSON pointer
  */
  public native any PtrGet(const char[] path);

  /**
  * Get boolean value by a JSON Pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  * @return                  boolean value referenced by the JSON pointer
  */
  public native bool PtrGetBool(const char[] path);

  /**
  * Get float value by a JSON Pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  * @return                  float value referenced by the JSON pointer
  */
  public native float PtrGetFloat(const char[] path);

  /**
  * Get integer value by a JSON Pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  * @return                  integer value referenced by the JSON pointer
  */
  public native int PtrGetInt(const char[] path);

  /**
  * Get integer64 value by a JSON Pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param buffer            Buffer to copy to
  * @param maxlength         Maximum size of the buffer
  *
  */
  public native bool PtrGetInt64(const char[] path, char[] buffer, int maxlength);

  /**
  * Get string value by a JSON Pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param buffer            Buffer to copy to
  * @param maxlength         Maximum size of the buffer
  *
  */
  public native bool PtrGetString(const char[] path, char[] buffer, int maxlength)

  /**
  * Get value is null by a JSON Pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  */
  public native bool PtrGetIsNull(const char[] path)

  /**
  * Set value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSet(const char[] path, YYJSON value);

  /**
  * Set boolean value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The boolean value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSetBool(const char[] path, bool value);

  /**
  * Set float value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The float value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSetFloat(const char[] path, float value);

  /**
  * Set integer value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The integer value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSetInt(const char[] path, int value);

  /**
  * Set intger64 value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The intger64 value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSetInt64(const char[] path, const char[] value);

  /**
  * Set string value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The string value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSetString(const char[] path, const char[] value);

  /**
  * Set null value by a JSON Pointer
  *
  * @note                    The parent nodes will be created if they do not exist. If the target value already exists, it will be replaced by the new value
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The null value to be set, pass NULL to remove
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrSetNull(const char[] path);

  /**
  * Add (insert) value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The value to be added
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAdd(const char[] path, YYJSON value);

  /**
  * Add (insert) boolean value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The boolean value to be added
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAddBool(const char[] path, bool value);

  /**
  * Add (insert) float value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The float value to be added
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAddFloat(const char[] path, float value);

  /**
  * Add (insert) integer value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The int value to be added
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAddInt(const char[] path, int value);

  /**
  * Add (insert) integer64 value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The intger64 value to be added
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAddInt64(const char[] path, const char[] value);

  /**
  * Add (insert) string value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  * @param value             The str value to be added
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAddString(const char[] path, const char[] value);

  /**
  * Add (insert) null value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  * @return                  true if JSON pointer is valid and new value is set, false otherwise
  */
  public native bool PtrAddNull(const char[] path);

  /**
  * Remove value by a JSON pointer
  *
  * @param path              The JSON pointer string (UTF-8 with null-terminator)
  *
  * @return                  true if removed value, false otherwise
  */
  public native bool PtrRemove(const char[] path)

  /**
  * Retrieves json type
  */
  property YYJSON_TYPE Type {
    public native get();
  }

  /**
  * Retrieves json subtype
  */
  property YYJSON_SUBTYPE SubType {
    public native get();
  }

  /**
  * Retrieves json is array
  */
  property bool IsArray {
    public native get();
  }

  /**
  * Retrieves json is object
  */
  property bool IsObject {
    public native get();
  }
};

methodmap YYJSONObject < YYJSON
{
  /**
  * Creates a JSON object A JSON object maps strings (called "keys") to values Keys in a
  * JSON object are unique That is, there is at most one entry in the map for a given key
  *
  * @note Needs to be freed using delete or CloseHandle()
  */
  public native YYJSONObject();
  
  /**
  * Loads a JSON object from a file
  *
  * @param file              File to read from
  * @param flag              The JSON read options
  * 
  * @return                  Object handle, or null on failure
  */
  public static native YYJSONObject FromFile(const char[] file, YYJSON_READ_FLAG flag = YYJSON_READ_NOFLAG);

  /**
  * Loads a JSON object from a string
  *
  * @param buffer            String buffer to load into the JSON object
  * @param flag              The JSON read options
  * 
  * @return                  Object handle, or null on failure
  */
  public static native YYJSONObject FromString(const char[] buffer, YYJSON_READ_FLAG flag = YYJSON_READ_NOFLAG);

  /**
  * Gets a value from the object
  * 
  * @note                    This function takes a linear search time
  *
  * @param key               Key name
  *
  * @return                  Returns the value to which the specified key is mapped, or null if this object contains no mapping for the key
  */
  public native any Get(const char[] key);

  /**
  * Sets a value in the object
  *
  * @param key               Key name
  * @param value             JSON handle to set
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool Set(const char[] key, const YYJSON value);

  /**
  * Gets a boolean value from the object
  *
  * @param key               Key name
  *
  * @return                  Boolean value
  */
  public native bool GetBool(const char[] key);

  /**
  * Gets a float value from the object
  *
  * @param key               Key name
  *
  * @return                  Float value
  */
  public native float GetFloat(const char[] key);

  /**
  * Gets a integer value from the object
  *
  * @param key               Key name
  *
  * @return                  Integer value
  */
  public native int GetInt(const char[] key);

  /**
  * Retrieves a 64-bit integer value from the object
  *
  * @param key               Key string
  * @param buffer            String buffer to store value
  * @param maxlength         Maximum length of the string buffer
  * 
  * @return                  True on success, false if the key was not found
  */
  public native bool GetInt64(const char[] key, char[] buffer, int maxlength);

  /**
  * Gets name of the object's key
  *
  * @param index             Position from which get key name
  * @param buffer            Buffer to copy string to
  * @param maxlength         Maximum size of the buffer
  *
  * @return                  True on success, false on failure
  */
  public native bool GetKeyName(int index, char[] buffer, int maxlength);

  /**
  * Gets a value at the specified position from the object
  *
  * @note                    Needs to be freed using delete or CloseHandle()
  *
  * @param index             Position from which get key name
  *
  * @return                  Returns the value to which index
  */
  public native any GetValueAt(int index);

  /**
  * Returns whether or not a key exists in the object
  *
  * @param key               Key string
  * @param ptr_use           Use JSON Pointer
  * 
  * @return                  True if the key exists, false otherwise
  */
  public native bool HasKey(const char[] key, bool ptr_use = false);

  /**
  * Replaces all matching keys with the new key
  * The key and new_key should be a null-terminated UTF-8 string
  * The new_key is copied and held by doc
  * 
  * @note                    This function takes a linear search time
  *
  * @param key               Key string
  * @param new_key           New key set
  * 
  * @return                  True if at least one key was renamed, false otherwise
  */
  public native bool RenameKey(const char[] key, const char[] new_key);

  /**
  * Gets string data from the object
  *
  * @param key               Key name
  * @param buffer            Buffer to copy string to
  * @param maxlength         Maximum size of the buffer
  *
  * @return                  True on success, false on failure
  */
  public native bool GetString(const char[] key, char[] buffer, int maxlength);

  /**
  * Returns whether or not a value in the object is null
  *
  * @param key               Key string
  *
  * @return                  True if the value is null, false otherwise
  */
  public native bool IsNull(const char[] key);

  /**
  * Sets a boolean value in the object
  *
  * @param key               Key name
  * @param value             Boolean value to set
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetBool(const char[] key, bool value);

  /**
  * Sets a float value in the object
  *
  * @param key               Key name
  * @param value             float to set
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetFloat(const char[] key, float value);

  /**
  * Sets a integer value in the object
  *
  * @param key               Key name
  * @param value             integer to set
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetInt(const char[] key, int value);

  /**
  * Sets a 64-bit integer value in the object, either inserting a new entry or replacing an old one
  *
  * @param key               Key string
  * @param value             Value to store at this key
  * 
  * @return                  True on success, false on failure
  */
  public native bool SetInt64(const char[] key, const char[] value);

  /**
  * Sets a null in the object
  *
  * @param key               Key name
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetNull(const char[] key);

  /**
  * Sets string data in the object
  *
  * @param key               Key name
  * @param value             String to copy
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetString(const char[] key, const char[] value);

  /**
  * Removes a key and its value in the object
  *
  * @note                    This function takes a linear search time
  *
  * @param key               Key name
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool Remove(const char[] key);

  /**
  * Removes all keys and their values in the object
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool Clear();
  
  /**
  * Retrieves the size of the object
  */
  property int Size {
    public native get();
  }
};

methodmap YYJSONArray < YYJSON
{
  /**
  * Creates a JSON array
  *
  * @note Needs to be freed using delete or CloseHandle()
  */
  public native YYJSONArray();

  /**
  * Loads a JSON array from a file
  *
  * @param file              File to read from
  * @param flag              Read flag
  * @return                  Array handle, or null on failure
  */
  public static native YYJSONArray FromFile(const char[] file, YYJSON_READ_FLAG flag = YYJSON_READ_NOFLAG);

  /**
  * Loads a JSON array from a string
  *
  * @param buffer            String buffer to load into the JSON array
  * @param flag              Read flag
  * @return                  Array handle, or null on failure
  */
  public static native YYJSONArray FromString(const char[] buffer, YYJSON_READ_FLAG flag = YYJSON_READ_NOFLAG);

  /**
  * Gets a value from the array
  * 
  * @note                    This function takes a linear search time
  *
  * @param index             Position in the array (starting from 0)
  *
  * @return                  NULL if array is NULL/empty or the index is out of bounds
  */
  public native any Get(int index);
  
  /**
  * Gets a boolean value from the array
  *
  * @param index             Position in the array (starting from 0)
  *
  * @return                  Boolean value
  */
  public native bool GetBool(int index);

  /**
  * Gets a float value from the array
  *
  * @param index             Position in the array (starting from 0)
  *
  * @return                  The number as float
  */
  public native float GetFloat(int index);

  /**
  * Gets a integer value from the array
  *
  * @param index             Position in the array (starting from 0)
  *
  * @return                  integer value
  */
  public native int GetInt(int index);

  /**
  * Gets a 64-bit integer from the array
  *
  * @param index             Position in the array (starting from 0)
  * @param buffer            Buffer to copy to
  * @param maxlength         Maximum size of the buffer
  *
  * @return                  64-bit integer
  */
  public native void GetInt64(int index, char[] buffer, int maxlength);

  /**
  * Gets string data from the array
  *
  * @param index             Position in the array (starting from 0)
  * @param buffer            Buffer to copy string to
  * @param maxlength         Maximum size of the buffer
  *
  * @return                  True on success, false on failure
  */
  public native bool GetString(int index, char[] buffer, int maxlength);

  /**
  * Returns whether or not a value in the array is null
  *
  * @param index             Position in the array (starting from 0)
  *
  * @return                  True if the value is null, false otherwise
  */
  public native bool IsNull(int index);

  /**
  * Replaces a value at index
  * 
  * @note                    This function takes a linear search time
  *
  * @param index             The index to which to replace the value
  * @param value             The new value to replace
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool Set(int index, const YYJSON value);

  /**
  * Replaces a boolean value at index
  *
  * @param index             The index to which to replace the value
  * @param value             The new boolean value to replace
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetBool(int index, bool value);

  /**
  * Replaces a float value at index
  *
  * @param index             The index to which to replace the value
  * @param value             The new float value to replace
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetFloat(int index, float value);

  /**
  * Replaces a integer value at index
  *
  * @param index             The index to which to replace the value
  * @param value             The new int value to replace
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetInt(int index, int value);

  /**
  * Replaces a intger64 value at index
  *
  * @param index             The index to which to replace the value
  * @param value             The new intger64 value to replace
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetInt64(int index, const char[] value);

  /**
  * Replaces a string value at index
  *
  * @param index             The index to which to replace the value
  * @param value             The new string value to replace
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetString(int index, const char[] value);

  /**
  * Replaces a null value at index
  *
  * @param index             The index to which to replace the value
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool SetNull(int index);

  /**
  * Inserts a value at the end of the array
  *
  * @param value             JSON handle to set
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool Push(const YYJSON value);

  /**
  * Inserts a boolean value at the end of the array
  *
  * @param value             Boolean value to set
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool PushBool(bool value);

  /**
  * Inserts a float value at the end of the array
  *
  * @param value             float to set
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool PushFloat(float value);

  /**
  * Inserts a integer value at the end of the array
  *
  * @param value             integer to set
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool PushInt(int value);

  /**
  * Inserts a intger64 value at the end of the array
  *
  * @param value             intger64 value
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool PushInt64(const char[] value);

  /**
  * Inserts a string value at the end of the array
  *
  * @param value             String to copy
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool PushString(const char[] value);

  /**
  * Inserts a null value at the end of the array
  *
  * @return                  The value to be inserted. Returns false if it is NULL
  */
  public native bool PushNull();

  /**
  * Removes an element from the array
  * 
  * @note                    This function takes a linear search time
  *
  * @param index             Position in the array (starting from 0)
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool Remove(int index);

  /**
  * Removes the first value in this array
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool RemoveFirst();

  /**
  * Removes and returns the last value in this array
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool RemoveLast();

  /**
  * Removes all values within a specified range in the array
  * 
  * @note                    This function takes a linear search time
  *
  * @param start_index       The start index of the range (0 is the first)
  * @param end_index         The number of items in the range (can be 0, but do nothing)
  * 
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool RemoveRange(int start_index, int end_index);

  /**
  * Removes all elements from the array
  *
  * @return                  True if succeed, false otherwise
  */
  public native bool Clear();

  /**
  * Retrieves the size of the array
  */
  property int Length {
    public native get();
  }

  /**
  * @note This function takes a linear search time
  * Returns the first element of this array
  * Returns NULL if arr is NULL/empty or type is not array
  * Needs to be freed using delete or CloseHandle()
  */
  property YYJSON First {
    public native get();
  }

  /**
  * @note This function takes a linear search time 
  * Returns the last element of this array
  * Returns NULL if arr is NULL/empty or type is not array
  * Needs to be freed using delete or CloseHandle()
  */
  property YYJSON Last {
    public native get();
  }
};