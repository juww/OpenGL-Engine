#include "scene.h"

#define INPUTFILE  freopen("input.txt","r",stdin)
#define OUTPUTFILE freopen("output.txt","w",stdout)

int main() {

    Scene* scene = Scene::getInstance();
    scene->run();

    return 0;
}