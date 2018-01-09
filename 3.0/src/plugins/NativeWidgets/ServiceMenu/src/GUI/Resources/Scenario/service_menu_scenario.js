/* @file �������� ���������� ���� ���������. */

//------------------------------------------------------------------------------
// ������������� ��������.
function initialize(scenarioName)
{
    // ���������
	ScenarioEngine.addState("main", {initial:true});
	ScenarioEngine.addState("done", {final:true});
	
    // �������� ����� �����������
	ScenarioEngine.addTransition("main", "done", "close");
}

//------------------------------------------------------------------------------
// ����� ��������.
function onStart() {
}

// ���������� ��������.
function onStop(aParams) {
}

// �������� �� ���������.
function canStop() {
	Core.graphics.notify(EventType.TryStopScenario, {});

	return false;
}

//------------------------------------------------------------------------------
// ����������� ���������.
function mainEnterHandler() {
    Core.graphics.show("ServiceMenu", { reset: true });
}