#ifndef __BOXEDWINE_DATA_H__
#define __BOXEDWINE_DATA_H__

class BoxedContainer;

class BoxedwineData {
public:
    static void init(int argc, const char **argv);
    static void startApp();
    static void loadUI();

    static const std::vector<BoxedContainer*> getContainers() {return BoxedwineData::containers;}
    static void addContainer(BoxedContainer* container) {BoxedwineData::containers.push_back(container);}
    static BoxedContainer* getContainerByName(const std::string& name);
    static void reloadContainers();

private:
    static void loadContainers();
    static std::vector<BoxedContainer*> containers;
};
#endif