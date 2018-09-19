/* @file Всплывающее окно */

import QtQuick 1.0
import Application.Types 1.0

Rectangle {
	id: rootItem
	
	width: 900
	height: 400
	color: "#f7f5ed"
	radius: 10
	border { width: 4; color: "#a6a6a6" }
	opacity: 1

	Image {
		id: imageMain
		
		x: 44
		y: 50
		sourceSize.height: 188
		sourceSize.width: 188
		fillMode: Image.PreserveAspectFit
		source: ""
	}

	AnimatedImage {
		id: imageWait
		
		x: 44
		y: 50
		source: "qrc:/MessageBox/wait.gif"
		visible: false
	}

	Text {
		id: textMessage
		
		x: 271
		y: 50
		width: 592
		height: paintedHeight
		color: "#000000"
		text: ""
		horizontalAlignment: Text.AlignHCenter
		wrapMode: Text.WordWrap
		font.pixelSize: 30
	}
	
	Flickable {
	    id: flick
	
			x: 271
			width: 592
			height: 200;
	    contentWidth: textMessageExt.paintedWidth
	    contentHeight: textMessageExt.paintedHeight
	    clip: true
			anchors.top: textMessage.bottom
	
	    function ensureVisible(r)
	    {
	        if (contentX >= r.x)
					{
	            contentX = r.x;
					}
	        else if (contentX+width <= r.x+r.width)
					{
	            contentX = r.x+r.width-width;
					}
	        if (contentY >= r.y)
					{
	            contentY = r.y;
					}
	        else if (contentY+height <= r.y+r.height)
					{
	            contentY = r.y+r.height-height;
					}
	    }
	
	    TextEdit {
	        id: textMessageExt
					
	        width: flick.width
	        height: flick.height       
	        wrapMode: TextEdit.Wrap
	        text: ""
					font.pixelSize: 20
					horizontalAlignment: Text.AlignHCenter
					visible: false
					readOnly: true
					onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
	    }    
	}	
	
	Image {
		id: btnCancel
		
		x: 606
		y: 264
		source: ""
		
		Text {
			id: btnText
			
			text: "#btn_caption"
			anchors { left: parent.left; top: parent.top; }
			width: parent.width
			height: parent.height
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
			color: "#ffffff";
			font { family: "PT Sans Caption"; pointSize: 21; }			
		}
		
		MouseArea {
			anchors.fill: parent
			onClicked: {
				if (global.closeWindow) {
					Application.graphics.hidePopup();
					global.closeWindow = false;			
				}
				else {
					Application.postEvent(EventType.UpdateScenario, {signal: "popup_notify"});
				}
			}
		}
	}

	Image {
		id: btnOK
		
		x: 271
		y: 264
		source: "qrc:/MessageBox/ok.png"

		MouseArea {
			anchors.fill: parent
			onClicked: {
				textMessageExt.visible = false;
				Application.graphics.hidePopup({button: MessageBox.OK});
				Application.postEvent(EventType.UpdateScenario, {signal: "popup_notify"});
			}
		}
	}

	QtObject {
		id: global

		property bool closeWindow: false
	}

	function notifyHandler(aEvent, aParameters)
	{
		if (aParameters["button_type"] == MessageBox.Text)
		{			
			btnText.visible = true;
			btnText.text = aParameters["button_text"];
			btnCancel.source = "qrc:/MessageBox/btn_empty.png";
		}
		else
		{
			if (aParameters["text_append_mode"])
			{
				textMessageExt.text += aParameters["text_message_ext"] + "\n";
				textMessageExt.cursorPosition = textMessageExt.text.length;
			}
			else
			{
				textMessageExt.text = aParameters["text_message_ext"];
			}

			textMessageExt.visible = true;
		}
	}

	function resetHandler(aParameters)
	{
		btnOK.visible = false;
		btnCancel.visible = false;
		btnText.visible = false;
		btnCancel.source = "qrc:/MessageBox/cancel.png"
		imageMain.visible = false;
		imageWait.visible = false;
		textMessage.text = aParameters["text_message"];
		textMessageExt.text = "";

		if (aParameters["icon"] == MessageBox.Question)
		{
			btnOK.pos.x = 271;
			btnOK.pos.y = 264;

			btnOK.visible = true;
			btnCancel.visible = true;

			imageMain.source = "qrc:/MessageBox/question.png";
			imageMain.visible = true;
			
			global.closeWindow = true;
		}
		else
		{
			btnOK.pos = btnCancel.pos;

			if (aParameters["icon"] == MessageBox.Wait)
			{
				imageMain.source = "qrc:/MessageBox/wait.mng";
				imageWait.visible = true;

				if (aParameters["button"] == MessageBox.Cancel)
				{
					btnCancel.visible = true;
					btnCancel.source = "qrc:/MessageBox/cancel_t.png";
				}

				return;
			}
			else if (aParameters["icon"] == MessageBox.Info)
			{
				imageMain.source = "qrc:/MessageBox/info.png";
			}
			else if (aParameters["icon"] == MessageBox.Warning)
			{
				imageMain.source = "qrc:/MessageBox/warning.png";
			}
			else if (aParameters["icon"] == MessageBox.Critical)
			{
				imageMain.source = "qrc:/MessageBox/critical.png";
			}

			imageMain.visible = true;

			if (aParameters["button"] == MessageBox.OK)
			{
				btnOK.visible = true;
			}
		}
	}
}
