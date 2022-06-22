//
//  PlaySwiftUI4.swift
//  PlayWWDC22
//
//  Created by Ming Dai on 2022/6/22.
//
import StoreKit
import SwiftUI

struct PlaySwiftUI4: View {
    var body: some View {
        PPPP()
        PPasteButton()
//        PMultiDatePicker()
//        PRequestReview()
//        PShareLink()
//        PTapLocation()
//        PGrid()
//        PSheet()
    }
}

struct PPPP: View {
    let colors: [Color] = [.blue, .cyan, .green, .yellow, .orange, .red, .purple]

        var body: some View {
            VStack {
                ForEach(colors, id: \.self) { color in
                    Rectangle().fill(color.gradient)
                }
            }
        }
}

struct PPasteButton: View {
    @State private var s = "戴铭"
    var body: some View {
        TextField("输入", text: $s)
            .textFieldStyle(.roundedBorder)
        PasteButton(payloadType: String.self) { str in
            guard let first = str.first else { return }
            s = first
        }
    }
}

struct PMultiDatePicker: View {
    @Environment(\.calendar) var cal
    @State var dates: Set<DateComponents> = []
    var body: some View {
        MultiDatePicker("选择个日子", selection: $dates, in: Date.now...)
        Text(s)
    }
    var s: String {
        dates.compactMap { c in
            cal.date(from:c)?.formatted(date: .long, time: .omitted)
        }
        .formatted()
    }
}

struct PRequestReview: View {
    @Environment(\.requestReview) var rr
    var body: some View {
        Button("来评论吧") {
            rr()
        }
    }
}

struct PShareLink: View {
    let url = URL(string: "https://ming1016.github.io/")!
    var body: some View {
        ShareLink(item: url, message: Text("戴铭的博客"))
        ShareLink("戴铭的博客", item: url)
        ShareLink(item: url) {
            Label("戴铭的博客", systemImage: "swift")
        }
    }
}

struct PTapLocation: View {
    var body: some View {
        Rectangle()
            .fill(.green)
            .frame(width: 50, height: 50)
            .onTapGesture(coordinateSpace: .global) { location in
                print("Tap in \(location)")
            }
    }
}

struct PGrid: View {
    var body: some View {
        Grid {
            GridRow {
                Text("One")
                Text("One")
                Text("One")
            }
            GridRow {
                Text("Two")
                Text("Two")
            }
            Divider()
            GridRow {
                Text("Three")
                Text("Three")
                    .gridCellColumns(2)
            }
        }
        
        // 棋盘
        Grid(horizontalSpacing: 0, verticalSpacing: 0) {
                    ForEach(0..<8) { row in
                        GridRow {
                            ForEach(0..<8) { col in
                                if (row + col).isMultiple(of: 2) {
                                    Rectangle()
                                        .fill(.black)
                                } else {
                                    Rectangle()
                                        .fill(.white)
                                }
                            }
                        }
                    }
                }
                .aspectRatio(1, contentMode: .fit)
                .border(.black, width: 1)
                .padding()
    }
}

struct PSheet: View {
    @State private var isShow = false
    var body: some View {
        Button("显示 Sheet") {
            isShow.toggle()
        }
        .sheet(isPresented: $isShow) {
            Text("这里是 Sheet 的内容")
                .presentationDetents([.medium, .large])
                .presentationDetents([.fraction(0.7)])
                .presentationDetents([.height(100)])
        }
    }
}
