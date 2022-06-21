//
//  PlaySyntax.swift
//  PlayWWDC22
//
//  Created by Ming Dai on 2022/6/21.
//

import Foundation

class PlaySyntax {
    
    static func generic() {
        func suffledArray<T: Sequence>(from options: T = 1...100) -> [T.Element] {
            Array(options.shuffled())
        }
        
        print(suffledArray())
        print(suffledArray(from: ["one", "two", "three"]))
        
        func isSorted(array: [some Comparable]) -> Bool {
            array == array.sorted()
        }
    }
    
    static func regex() {
        let s1 = "I am not a good painter"
        print(s1.ranges(of: /good/))
        do {
            let regGood = try Regex("[a-z]ood")
            print(s1.replacing(regGood, with: "bad"))
        } catch {
            print(error)
        }
        print(s1.trimmingPrefix(/i am /.ignoresCase()))
        
        let reg1 = /(.+?) read (\d+) books./
        let reg2 = /(?<name>.+?) read (?<books>\d+) books./
        let s2 = "Jack read 3 books."
        do {
            if let r1 = try reg1.wholeMatch(in: s2) {
                print(r1.1)
                print(r1.2)
            }
            if let r2 = try reg2.wholeMatch(in: s2) {
                print("name:" + r2.name)
                print("books:" + r2.books)
            }
        } catch {
            print(error)
        }
        
        
            
    }
    
    static func clockInstantDuration() async {
        print(Date())
        do {
            try await Task.sleep(until: .now + .seconds(3), clock: .continuous)
        } catch {
            print(error)
        }
        
    }
    
    static func multiStatementClosureTypeInference() {
        let a = [1,2,3]
        let r = a.map { i in
            if i >= 2 {
                return "\(i) 大于等于2"
            } else {
                return "\(i) 小于2"
            }
        }
        print(r)
    }
    
    static func ifletOptional() {
        let s: String? = "hi"
        if let s {
            print(s)
        }
    }
    
}
