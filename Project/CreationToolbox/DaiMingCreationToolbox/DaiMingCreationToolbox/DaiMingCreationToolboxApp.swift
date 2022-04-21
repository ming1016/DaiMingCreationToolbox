//
//  DaiMingCreationToolboxApp.swift
//  DaiMingCreationToolbox
//
//  Created by Ming Dai on 2022/4/21.
//

import SwiftUI


final class DaiMingCreationToolboxApplication: NSApplication {
    let strongDelegate = AppDelegate()
    
    override init() {
        super.init()
        self.delegate = strongDelegate
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

@NSApplicationMain
final class AppDelegate: NSObject, NSApplicationDelegate, ObservableObject {
    func applicationWillFinishLaunching(_ notification: Notification) {
        //
        print("ok")
    }
    
    func applicationDidFinishLaunching(_ notification: Notification) {
        print("did")
    }
}


