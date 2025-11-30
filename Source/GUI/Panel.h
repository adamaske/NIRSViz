#pragma once


class IPanel {
public:
	virtual ~IPanel() = default;

	virtual void OnGUIRender(bool standalone, bool& open) = 0;
};

class ImGuiPanel : public IPanel {
public:
	virtual void OnImGUIRender(bool standalone, bool& open) = 0;

	virtual void OnGUIRender(bool standalone, bool& open) override {
		OnImGUIRender(standalone, open);
	};
};


