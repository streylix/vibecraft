#include <gtest/gtest.h>

#include "vibecraft/weather.h"

// M23: Weather System

using vibecraft::BiomeType;
using vibecraft::WeatherState;
using vibecraft::WeatherSystem;

TEST(Weather, DefaultClear) {
    WeatherSystem weather;
    EXPECT_EQ(weather.GetWeather(), WeatherState::kClear);
}

TEST(Weather, RainState) {
    WeatherSystem weather;
    weather.SetWeather(WeatherState::kRain);
    EXPECT_EQ(weather.GetWeather(), WeatherState::kRain);
}

TEST(Weather, SnowState) {
    WeatherSystem weather;
    weather.SetWeather(WeatherState::kSnow);
    EXPECT_EQ(weather.GetWeather(), WeatherState::kSnow);
}

TEST(Weather, BiomeDesertNoRain) {
    WeatherSystem weather;
    weather.SetWeather(WeatherState::kRain);
    EXPECT_EQ(weather.GetWeather(), WeatherState::kRain);

    // Desert should always report Clear regardless of global weather.
    WeatherState desert_weather = weather.GetWeatherForBiome(BiomeType::kDesert);
    EXPECT_EQ(desert_weather, WeatherState::kClear);

    // Also check with snow.
    weather.SetWeather(WeatherState::kSnow);
    desert_weather = weather.GetWeatherForBiome(BiomeType::kDesert);
    EXPECT_EQ(desert_weather, WeatherState::kClear);
}

TEST(Weather, BiomeTundraSnow) {
    WeatherSystem weather;
    weather.SetWeather(WeatherState::kRain);

    // Tundra should convert rain to snow.
    WeatherState tundra_weather = weather.GetWeatherForBiome(BiomeType::kTundra);
    EXPECT_EQ(tundra_weather, WeatherState::kSnow);

    // If global weather is already snow, tundra should still be snow.
    weather.SetWeather(WeatherState::kSnow);
    tundra_weather = weather.GetWeatherForBiome(BiomeType::kTundra);
    EXPECT_EQ(tundra_weather, WeatherState::kSnow);

    // If global weather is clear, tundra should be clear.
    weather.SetWeather(WeatherState::kClear);
    tundra_weather = weather.GetWeatherForBiome(BiomeType::kTundra);
    EXPECT_EQ(tundra_weather, WeatherState::kClear);
}
