#include "json.h"

int main(int argc, char const* argv[]) {
    json jsn = json::array(1, "test", 3, "呵呵");
    printf("%s\n", jsn.dump().c_str());
    printf("%s\n", jsn[1].get<std::string>().c_str());
    
    json bjsn;
    bjsn["a"] = 1;
    bjsn["b"].append(1);
    bjsn["arr"] = jsn;
    printf("%s\n", bjsn.dump().c_str());
    
    jsn = bjsn;
    printf("%s\n", jsn.dump().c_str());
    return 0;
}
