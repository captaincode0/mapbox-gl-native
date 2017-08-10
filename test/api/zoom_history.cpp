#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_file_source.hpp>

#include <mbgl/gl/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/renderer/backend_scope.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/default_thread_pool.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/geojson.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>

using namespace mbgl;
using namespace mbgl::style;

TEST(API, ZoomHistory) {
    util::RunLoop loop;

    StubFileSource fileSource;
    ThreadPool threadPool(4);
    float pixelRatio { 1 };

    HeadlessFrontend frontend { pixelRatio, fileSource, threadPool };
    auto map = std::make_unique<Map>(frontend, MapObserver::nullObserver(), frontend.getSize(),
                                     pixelRatio, fileSource, threadPool, MapMode::Still);

    EXPECT_TRUE(map);

    auto loadStyle = [&](auto style) {
        auto source = std::make_unique<GeoJSONSource>("geometry");
        source->setGeoJSON({ LineString<double> { { -45, -45 }, { 45, 45 } } });

        auto layer = std::make_unique<LineLayer>("geometry", "geometry");
        layer->setLineWidth(4.0);
        layer->setLineDasharray({ std::vector<float> { 1, 1 } });

        map->getStyle().loadJSON(util::read_file(style));
        map->getStyle().addSource(std::move(source));
        map->getStyle().addLayer(std::move(layer));
    };

    loadStyle("test/fixtures/api/empty.json");
    EXPECT_DOUBLE_EQ(map->getZoom(), 0.0);
    test::checkImage("test/fixtures/zoom_history/normal", frontend.render(*map), 0.0006, 0.1);

    loadStyle("test/fixtures/api/empty-zoomed.json");
    EXPECT_DOUBLE_EQ(map->getZoom(), 0.5);
    test::checkImage("test/fixtures/zoom_history/zoomed", frontend.render(*map), 0.0006, 0.1);
}
