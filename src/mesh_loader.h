
#include "model.h"

#include <string>
#include <vector>
namespace vt {


class ModelLoader {
public:
    virtual ~ModelLoader() = default;

    virtual bool loadModel(const std::string& path, std::vector<VtModel::Vertex>& vertices, std::vector<uint32_t>& indices) = 0;

};

}
