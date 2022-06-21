//
//  PlayWeatherKit.swift
//  PlayWWDC22
//
//  Created by Ming Dai on 2022/6/21.
//
import WeatherKit
import CoreLocation

class PlayWeatherKit {
    
    func simpleShowWeather() async {
        do {
            let wts = WeatherService()
            let place = CLLocation(latitude: 39.9042, longitude: 116.4074)
            let wt = try await wts.weather(for: place)
            let temperature = wt.currentWeather.apparentTemperature
            
            let (minute, alerts) = try await wts.weather(for: place, including: .minute, .alerts)
        } catch {
            print(error)
        }
    }
}
