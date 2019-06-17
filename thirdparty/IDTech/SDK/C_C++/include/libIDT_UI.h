#ifndef LIBIDT_UI_H_
#define LIBIDT_UI_H_

#include "IDTDef.h"

/**
  * Reset to Initial State
  *  This command places the reader UI into the idle state and displays the appropriate idle display.
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_resetInitialState();

/**
  * Custom Display Mode
  *  Controls the LCD display mode to custom display. Keyboard entry is limited to the Cancel, Clear,
  *  Enter and the function keys, if present. PIN entry is not permitted while the
  *  reader is in Custom Display Mode
  *
  * @param enable TRUE = enabled, FALSE = disabled
  *
  * @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
  */
int ui_customDisplayMode(IN int enable);

/**
  * Set Foreground and Background Color
  *  This command sets the foreground and background colors of the LCD.
  *
  * @param foreRGB Foreground RGB. 000000 = black, FF0000 = red, 00FF00 = green, 0000FF = blue, FFFFFF = white
  * @param Length of foreRGB.  Must be 3.
  * @param backRGB Background RGB. 000000 = black, FF0000 = red, 00FF00 = green, 0000FF = blue, FFFFFF = white
  * @param Length of backRGB.  Must be 3.
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_setForeBackColor(IN BYTE *foreRGB, IN int foreRGBLen, IN BYTE *backRGB, IN int backRGBLen);

/**
  * Clear Display
  *  Command to clear the display screen on the reader.It returns the display to the currently defined background color and terminates all events
  *
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_clearDisplay();

/**
  * Enables Signature Capture
  *  This command executes the signature capture screen.  Once a signature is captured, it is sent to the callback
  *  with DeviceState.Signature, and the data will contain a .png of the signature
  *
  * @param timeout  Timeout waiting for the signature capture
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_captureSignature(IN int timeout);

/**
  * Start slide show
  *  You must send images to the reader�s memory and send a Start Custom Mode command to the reader before it will respond to this commands. Image files must be in .bmp or .png format.
  *
  * @param files Complete paths and file names of the files you want to use, separated by commas. If a directory is specified, all files in the dirctory are displayed
  * @param filesLen Length of files
  * @param posX X coordinate in pixels,   Range 0-271
  * @param posY Y coordinate in pixels,   Range 0-479
  * @param posMode Position Mode
  *  - 0 = Center on Line Y
  *  - 1 = Display at (X,Y)
  *  - 2 - Center on screen
  * @param touchEnable TRUE = Image is touch sensitive
  * @param recursion TRUE = Recursively follow directorys in list
  * @param touchTerminate TRUE = Terminate slideshow on touch (if touch enabled)
  * @param delay Number of seconds between image displays
  * @param loops  Number of display loops.  A zero indicates continuous display
  * @param clearScreen  TRUE = Clear screen
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_startSlideShow(IN char *files, IN int filesLen, IN int posX, IN int posY, IN int posMode, IN int touchEnable, IN int recursion, IN int touchTerminate, IN int delay, IN int loops, IN int clearScreen);
/**
  * Set Display Image
  *  You must send images to the reader�s memory and send a Start Custom Mode command to the reader before it will respond to Image commands. Image files must be in .bmp or .png format.
  *
  * @param file Complete path and file name of the file you want to use. Example "file.png" will put in root directory, while "ss/file.png" will put in ss directory (which must exist)
  * @param fileLen Length of files
  * @param posX X coordinate in pixels,   Range 0-271
  * @param posY Y coordinate in pixels,   Range 0-479
  * @param posMode Position Mode
  *  - 0 = Center on Line Y
  *  - 1 = Display at (X,Y)
  *  - 2 - Center on screen
  * @param touchEnable TRUE = Image is touch sensitive
  * @param clearScreen  TRUE = Clear screen
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_setDisplayImage(IN char *file, IN int fileLen, IN int posX, IN int posY, IN int posMode, IN int touchEnable, IN int clearScreen);
/**
  * Set Background Image
  *  You must send images to the reader�s memory and send a Start Custom Mode command to the reader before it will respond to Image commands. Image files must be in .bmp or .png format.
  *
  * @param file Complete path and file name of the file you want to use. Example "file.png" will put in root directory, while "ss/file.png" will put in ss directory (which must exist)
  * @param fileLen Length of files
  * @param enable TRUE = Use Background Image, FALSE = Use Background Color
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ui_setBackgroundImage(IN char *file, IN int fileLen, IN int enable);
/**
 * Displays text.
 *
 * Custom Display Mode must be enabled for custom text.
 * PIN pad entry is not allowed in Custom Display Mode but the Cancel, OK, and Clear keys remain active.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param displayWidth Width of the display area in pixels (optional)
 * @param displayHeight Height of the display area in pixels (optional)
 * @param fontDesignation Font designation
 * 	1 - Default
 * @param fontID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param screenPosition Display position
 * 	0 - Center on line Y without clearing screen
 * 	1 - Center on line Y after clearing screen
 * 	2 - Display at (X, Y) without clearing screen
 * 	3 - Display at (X, Y) after clearing screen
 * 	4 - Display at center of screen without clearing screen
 * 	5 - Display at center of screen after clearing screen
 * 	6 - Display text right-justified without clearing screen
 * 	7 - Display text right-justified after clearing screen
 * @param displayText Display text (Maximum: 1900 characters)
 * @param graphicsID A four byte array containing the ID of the created element (optional)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_displayText(IN int posX, IN int posY, IN int displayWidth,
		IN int displayHeight, IN int fontDesignation, IN int fontID,
		IN int screenPosition, IN char *displayText, OUT BYTE *graphicsID);
/**
 * Displays text with scroll feature.
 *
 * Custom Display Mode must be enabled.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param displayWidth Width of the display area in pixels (Minimum: 40px)
 * 	0 or NULL - Use the full width to display text
 * @param displayHeight Height of the display area in pixels (Minimum: 100px)
 * 	0 or NULL - Use the full height to display text
 * @param fontDesignation Font designation
 * 	1 - Default
 * @param fontID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param displayProperties Display properties for the text
 *  0 - Center on line Y without clearing screen
 *  1 - Center on line Y after clearing screen
 *  2 - Display at (X, Y) without clearing screen
 *  3 - Display at (X, Y) after clearing screen
 *  4 - Center on screen without clearing screen
 *  5 - Center on screen after clearing screen
 * @param displayText Display text (Maximum: 3999 characters plus terminator)
 */
int ui_displayParagraph(IN int posX, IN int posY, IN int displayWidth,
		IN int displayHeight, IN int fontDesignation, IN int fontID,
		IN int displayProperties, IN char *displayText);
/**
 * Displays an interactive button.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param buttonWidth Width of the button
 * @param buttonHeight Height of the button
 * @param fontDesignation Font designation
 * 	1 - Default
 * @param Font ID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param displayPosition Button display position
 * 	0 - Center on line Y without clearing screen and without word wrap
 * 	1 - Center on line Y after clearing screen and without word wrap
 * 	2 - Display at (X, Y) without clearing screen and without word wrap
 * 	3 - Display at (X, Y) after clearing screen and without word wrap
 * 	4 - Center button on screen without clearing screen and without word wrap
 * 	5 - Center button on screen after clearing screen and without word wrap
 * 	64 - Center on line Y without clearing screen and with word wrap
 * 	65 - Center on line Y after clearing the screen and with word wrap
 * 	66 - Display at (X, Y) without clearing screen and with word wrap
 * 	67 - Display at (X, Y) after clearing screen and with word wrap
 * 	68 - Center button on screen without clearing screen and with word wrap
 * 	69 - Center button on screen after clearing screen and with word wrap
 * @param buttonLabel Button label text (Maximum: 31 characters)
 * @param buttonTextColorR - Red component for foreground color (0 - 255)
 * @param buttonTextColorG - Green component for foreground color (0 - 255)
 * @param buttonTextColorB - Blue component for foreground color (0 - 255)
 * @param buttonBackgroundColorR - Red component for background color (0 - 255)
 * @param buttonBackgroundColorG - Green component for background color (0 - 255)
 * @param buttonBackgroundColorB - Blue component for background color (0 - 255)
 * @param graphicsID A four byte array containing the ID of the created element (optional)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_displayButton(IN int posX, IN int posY, IN int buttonWidth,
		IN int buttonHeight, IN int fontDesignation, IN int fontID,
		IN int displayPosition, IN char *buttonLabel, IN int buttonTextColorR,
		IN int buttonTextColorG, IN int buttonTextColorB, IN int buttonBackgroundColorR,
		IN int buttonBackgroundColorG, IN int buttonBackgroundColorB, OUT BYTE *graphicsID);
/**
 * Creates a display list.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param numOfColumns Number of columns to display
 * @param numOfRows Number of rows to display
 * @param fontDesignation Font Designation
 * 	1 - Default font
 * @param fontID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param verticalScrollArrowsVisible Display vertical scroll arrows by default
 * @param borederedListItems Draw border around list items
 * @param borederedScrollArrows Draw border around scroll arrows (if visible)
 * @param touchSensitive List items are touch enabled
 * @param automaticScrolling Enable automatic scrolling of list when new items exceed display area
 * @param graphicsID A four byte array containing the ID of the created element (optional)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_createList(IN int posX, IN int posY, IN int numOfColumns,
		IN int numOfRows, IN int fontDesignation, IN int fontID,
		IN int verticalScrollArrowsVisible, IN int borderedListItems, IN int borderdScrollArrows,
		IN int touchSensitive, IN int automaticScrolling, OUT BYTE *graphicsID);
/**
 * Adds an item to an existing list.
 *
 * Custom Display Mode must be enabled for custom text.
 *
 * @param listGraphicsID Existing list's graphics ID (4 byte array) that is provided during creation
 * @param itemName Item name (Maximum: 127 characters)
 * @param itemID Identifier for the item (Maximum: 31 characters)
 * @param selected If the item should be selected
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_addItemToList(IN BYTE *listGraphicsID, IN char *itemName, IN char *itemID,
		IN int selected);
/**
 * Retrieves the selected item's ID.
 *
 * @param listGraphicsID Existing list's graphics ID (4 byte array) that is provided during creation
 * @param itemID The selected item's ID (Maximum: 32 characters)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_getSelectedListItem(IN BYTE *listGraphicsID, OUT char *itemID);

/**
 * Removes all entries from the event queue.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_clearEventQueue();

/**
 * Requests input from the reader.
 *
 * @param timeout Timeout amount in seconds
 * 	0 - No timeout
 * @param dataReceived Indicates if an event occurred and data was received
 * 	0 - No data received
 * 	1 - Data received
 * @param eventType The event type (required to be at least 4 bytes), see table below
 * @param graphicsID The graphicID of the event (required to be at least 4 bytes)
 * @param eventData The event data, see table below (required to be at least 73 bytes)
 *
 * | Event Type         | Value (4 bytes) | Event Specific Data
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Button Event       | 00030000h       | Length = Variable												    |
 * |				    |				  |	Byte 1: State (1 = Pressed, other values RFU)					    |
 * |					|				  |	Byte 2 - n: Null terminated caption 							    |
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Checkbox Event     | 00030001h       | Length = 1 byte													    |
 * |					|			      |	Byte 1: State (1 = Checked, 0 = Unchecked)						    |
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Line Item Event    | 00030002h       | Length = 5 bytes												    |
 * |					|				  |	Byte 1: State (1 = Item Selected, other values RFU)				    |
 * |					|				  |	Byte 2 - n: Caption of the selected item						    |
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Keypad Event       | 030003h         | Length - 3 bytes												    |
 * |					|				  | Byte 1: State (1 = key pressed, 2 = key released, other values RFU) |
 * |					|				  | Byte 2 - 3: Key pressed and Key release                             |
 * |					|				  |		0030h - KEYPAD_KEY_0											|
 * |					|				  |		0031h - KEYPAD_KEY_1											|
 * |					|				  |		0032h - KEYPAD_KEY_2											|
 * |					|				  |		0033h - KEYPAD_KEY_3											|
 * |					|				  |		0034h - KEYPAD_KEY_4											|
 * |					|				  |		0035h - KEYPAD_KEY_5											|
 * |					|				  |		0036h - KEYPAD_KEY_6											|
 * |					|				  |		0037h - KEYPAD_KEY_7											|
 * |					|				  |		0038h - KEYPAD_KEY_8											|
 * |					|				  |		0039h - KEYPAD_KEY_9											|
 * |					|				  | Byte 2 - 3: Only Key pressed										|
 * |					|				  | 	000Dh - KEYPAD_KEY_ENTER										|
 * |					|				  | 	0008h - KEYPAD_KEY_CLEAR										|
 * |					|				  | 	001Bh - KEYPAD_KEY_CANCEL										|
 * |					|				  | 	0070h - FUNC_KEY_F1 (Vend III)									|
 * |					|				  | 	0071h - FUNC_KEY_F2 (Vend III)									|
 * |					|				  | 	0072h - FUNC_KEY_F3 (Vend III)									|
 * |					|				  | 	0073h - FUNC_KEY_F4 (Vend III)									|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Touchscreen Event  | 030004h         | Length = 1 - 33 bytes												|
 * |					|				  | Byte 1: State (not used)											|
 * |					|				  | Byte 2 - 33: Image name (zero terminated)							|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Slideshow Event    | 030005h         | Length = 1 byte														|
 * |					|				  | Byte 1: State (not used)											|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Transaction Event  | 030006h         | Length = 9 bytes													|
 * |					|				  | Byte 1: State (not used)											|
 * |					|				  | Byte 2 - 5: Card type (0 = unknown)									|
 * |					|				  | Byte 6 - 9: Status - A four byte, big-endian field					|
 * |					|				  |	Byte 9 is used to store the 1-byte status code						|
 * |					|				  |		00 - SUCCESS													|
 * |					|				  |		08 - TIMEOUT													|
 * |					|				  |		0A - FAILED														|
 * |					|				  | This is not related to the extended status codes					|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Radio Button Event | 030007h         | Length = 73 bytes													|
 * |					|				  | Byte 1: State (1 = Change ins selected button, other values RFU)	|
 * |					|				  | Byte 2 - 33: Null terminated group name								|
 * |					|				  | Byte 34 - 65: Radio button caption									|
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ui_getInputEvent(IN int timeout, OUT int *dataReceived, OUT BYTE *eventType,
		OUT BYTE *graphicsID, OUT BYTE *eventData);


 /**
 * Control Indicators
 *
 * Control the reader.  If connected, returns success.  Otherwise, returns timeout.
 * @param indicator description as follows:
 * 00h: ICC LED
 * 01h: Blue MSR
 * 02h: Red MSR
 * 03h: Green MSR
 *
 * @param enable  TRUE = 1, FALSE = 0
 *
 *
 * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
 */
int ui_controlIndicator(IN int indicator, IN int enable);

/**
 * Control User Interface
 *
 * Controls the User Interface:  Display, Beep, LED
 *
 * @param values Four bytes to control the user interface
 * Byte[0] = LCD Message
 * Messages 00-07 are normally controlled by the reader.
 * - 00h: Idle Message (Welcome)
 * - 01h: Present card (Please Present Card)
 * - 02h: Time Out or Transaction cancel (No Card)
 * - 03h: Transaction between reader and card is in the middle (Processing...)
 * - 04h: Transaction Pass (Thank You)
 * - 05h: Transaction Fail (Fail)
 * - 06h: Amount (Amount $ 0.00 Tap Card)
 * - 07h: Balance or Offline Available funds (Balance $ 0.00) Messages 08-0B are controlled by the terminal
 * - 08h: Insert or Swipe card (Use Chip & PIN)
 * - 09h: Try Again(Tap Again)
 * - 0Ah: Tells the customer to present only one card (Present 1 card only)
 * - 0Bh: Tells the customer to wait for authentication/authorization (Wait)
 * - FFh: indicates the command is setting the LED/Buzzer only.
 * Byte[1] = Beep Indicator
 * - 00h: No beep
 * - 01h: Single beep
 * - 02h: Double beep
 * - 03h: Three short beeps
 * - 04h: Four short beeps
 * - 05h: One long beep of 200 ms
 * - 06h: One long beep of 400 ms
 * - 07h: One long beep of 600 ms
 * - 08h: One long beep of 800 ms
 * Byte[2] = LED Number
 * - 00h: LED 0 (Power LED) 01h: LED 1
 * - 02h: LED 2
 * - 03h: LED 3
 * - FFh: All LEDs
 * Byte[3] = LED Status
 * - 00h: LED Off
 * - 01h: LED On
 * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
 */
int ui_controlUserInterface(IN BYTE *values);

 /**
 * Set Buzzer/LED
 *
 *  Sets the readers buzzer and LED's.
 *
 *  @param buzzer
 *  - 00h:  No beep
 *  - 01h:  Single beep
 *  - 02h:  Double beep
 *  - 03h:  Three short beeps
 *  - 04h:  Four short beeps
 *  - 05h:  One long beep of 200 ms
 *  - 06h:  One long beep of 400 ms
 *  - 07h:  One long beep of 600 ms
 *  - 08h:  One long beep of 800 ms
 *  @param led
 *  - 00h:  LED 0 (Leftmost LED)
 *  - 01h:  LED 1
 *  - 02h:  LED 2
 *  - 03h:  LED 3
 *  - FFh:  All LEDs
 * @param ledON  TRUE = ON, FALSE = OFF
 *
 *
 *
 * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
 */
int ui_setBuzzerLED(OUT BYTE buzzer, OUT BYTE led, OUT int ledON);

#endif /* LIBIDT_UI_H_ */

/*! \file libIDT_UI.h
 \brief UI Handling Library.

 UI Library API methods.
 */

/*! \def IN
  INPUT parameter.
 */

/*! \def OUT
  OUTPUT parameter.
 */

/*! \def IN_OUT
  INPUT / OUTPUT PARAMETER.
 */

/*! \def _DATA_BUF_LEN
 DATA BUFFER LENGTH
 */
