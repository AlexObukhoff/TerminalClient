/* @file �������� ������������� ���������. */

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

//-----------------------------------------------------------------------------
// ����� ��������.
function onStart()
{
}

// ���������� ��������.
function onStop()
{
}

// �������� �� ���������.
function canStop()
{
	return false;
}

//-----------------------------------------------------------------------------
// ����������� ���������.
function mainEnterHandler(aParam)
{
	if (aParam.signal == "resume")
	{
		// �� ������ ���� �� ���������� ����, ������� �������.
		Core.postEvent(EventType.UpdateScenario, "close");
	}
	else
	{
		Core.graphics.show("AutoEncashment", {reset:true});
	}
}
