#pragma once

#ifdef NEMUEXTERNALRENDERERIPC_EXPORTS
#define EXTERNALRENDERERAPI __declspec(dllexport)
#else
#define EXTERNALRENDERERAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/*
	* @path: emulator install path
	* @index: multi-instance index num
	* @return: >0 when connect success, 0 when fail.
	*/
	EXTERNALRENDERERAPI int nemu_connect(const wchar_t* path, int index);

	/*
	* disconnect handle;
	*/
	EXTERNALRENDERERAPI void nemu_disconnect(int handle);

	/*
	* call this function twice to get valid pixels data.
	* first you set @buffer_size to 0, function will return valid width and heigth to @width and @height.
	* then you set 4*@width*@height to @buffer_size, and call this function again, @pixels will contain valid data when function success.
	* @handle: value returned from nemu_connect();
	* @displayid: display id, current only 0 is valid.
	* @buffer_size: ref above.
	* @width,@height: valid width and height.
	* @pixels: valid pixels data.
	* @return: >0 when connect success, 0 when fail.
	*/
	EXTERNALRENDERERAPI int nemu_capture_display(int handle, unsigned int displayid, int buffer_size, int *width, int *height, unsigned char* pixels);

	/*
    * send text to emulator.
	* @handle: return value from connect_server function.
	* @buf: text buffer
	* @isze: buffer size
	* return: 0 means success, > 0 means fail
	*/
	EXTERNALRENDERERAPI int nemu_input_text(int handle, int size, const char* buf);

	/*
    * press button.
	* @displayid: display id, current only 0 is valid.
	* @return: 0 means success, > 0 means fail.
	*/
	EXTERNALRENDERERAPI int nemu_input_event_touch_down(int handle, int displayid, int x_point, int y_point);

	/*
    * release button.
	* @return: 0 means success, > 0 means fail.
	*/
	EXTERNALRENDERERAPI int nemu_input_event_touch_up(int handle, int displayid);

	/*
    * press keyboard.
	* @key_code: ref in https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
	* @return: 0 means success, > 0 means fail.
	*/
	EXTERNALRENDERERAPI int nemu_input_event_key_down(int handle, int displayid, int key_code);

	/*
	* used to release keyboard event.
	* @return: 0 means success, > 0 means fail.
	*/
	EXTERNALRENDERERAPI int nemu_input_event_key_up(int handle, int displayid, int key_code);


#ifdef __cplusplus
}
#endif