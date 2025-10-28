
using namespace geode::prelude;

class SaveManager : public Popup<> {
public:
	bool setup() override;
	static SaveManager* create();

};
