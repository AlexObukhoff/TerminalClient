call %QTDIR%\bin\qtvars.bat

set LOCALE_DIR=locale
set LOPTIONS=-no-obsolete

for %%F in (
	main_menu_scene.qml
	search_scene.qml
	language_scene.qml
	addinfo_scene.qml
	confirm_payment_scene.qml
	denom_info_scene.qml
	edit_payment_scene.qml
	edit_pin_scene.qml
	pay_scene.qml
	result_scene.qml
	send_receipt_scene.qml
	html_popup.qml
	info_popup.qml
	payment_method_selector_scene.qml
	provider_info_popup.qml
	platru_login_scene.qml
	platru_menu_scene.qml
	payment_method_selector_scene.qml
	platru_edit_entry_scene.qml
	platru_fill_amount_scene.qml
	platru_history_scene.qml
	platru_select_provider_scene.qml
	user_assistant_scene.qml
	provider_selector_popup.qml
	topup_platru_scene.qml
	edit_amount_scene.qml
	"widgets\scene_base.qml"
	"widgets\scene_base2.qml"
	"widgets\status_bar.qml"
	"widgets\operator_selector.qml"
	"widgets\operator_menu.qml"
	"widgets\keyboard.qml"
	"info_content\terminal_info.qml"
	"scenario\payment_scenario.js"
	"scenario\cash_charge_scenario.js"
	"scenario\card_charge_scenario.js"
	"scenario\cash_dispense_scenario.js"
	"scenario\menu_scenario.js"
	"scripts\root_groups.js"
	"scripts\errors.js"
	"scene_with_context\operators\999\edit_mobile_payment_scene.qml"
) do (
        echo .
        echo "Update " %%F
	lupdate %LOPTIONS% "%%F" -ts "%LOCALE_DIR%\%%~nF_en.ts"
	lupdate %LOPTIONS% "%%F" -ts "%LOCALE_DIR%\%%~nF_ru.ts"
	lupdate %LOPTIONS% "%%F" -ts "%LOCALE_DIR%\%%~nF_kk.ts"
	lupdate %LOPTIONS% "%%F" -ts "%LOCALE_DIR%\%%~nF_de.ts"
)

lupdate %LOPTIONS% "info_scene.qml" "info_content\info_model.js" -ts "%LOCALE_DIR%\info_scene_en.ts"
lupdate %LOPTIONS% "info_scene.qml" "info_content\info_model.js" -ts "%LOCALE_DIR%\info_scene_ru.ts"
lupdate %LOPTIONS% "info_scene.qml" "info_content\info_model.js" -ts "%LOCALE_DIR%\info_scene_kk.ts"
lupdate %LOPTIONS% "info_scene.qml" "info_content\info_model.js" -ts "%LOCALE_DIR%\info_scene_de.ts"

set EDITORS="widgets\text_editor.qml" "widgets\number_editor.qml" "widgets\enum_editor.qml" "widgets\table_editor.qml" "widgets\fallback_editor.qml"

lupdate %LOPTIONS% %EDITORS% -ts "%LOCALE_DIR%\editor_en.ts"
lupdate %LOPTIONS% %EDITORS% -ts "%LOCALE_DIR%\editor_ru.ts"
lupdate %LOPTIONS% %EDITORS% -ts "%LOCALE_DIR%\editor_kk.ts"
lupdate %LOPTIONS% %EDITORS% -ts "%LOCALE_DIR%\editor_de.ts"

