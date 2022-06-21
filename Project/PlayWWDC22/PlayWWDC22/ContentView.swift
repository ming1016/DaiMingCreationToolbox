//
//  ContentView.swift
//  PlayWWDC22
//
//  Created by Ming Dai on 2022/6/21.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundColor(.accentColor)
            Text("Hello, world!")
        }
        .task {
            PlaySyntax.generic()
//            PlaySyntax.regex()
//            await PlaySyntax.clockInstantDuration()
//            PlaySyntax.multiStatementClosureTypeInference()
//            PlaySyntax.ifletOptional()
//            await PlayWeatherKit().simpleShowWeather()
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
