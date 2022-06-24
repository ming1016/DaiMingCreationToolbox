//
//  PlaySwiftUI4.swift
//  PlayWWDC22
//
//  Created by Ming Dai on 2022/6/22.
//
import StoreKit
import SwiftUI
import Charts

struct PlaySwiftUI4: View {
    var body: some View {
        
        PTextTransitionsView()
//        PButtonStyleComposition()
//        PGradientAndShadow()
//        PGauge()
//        PAnyLayout()
//        PViewThatFits()
//        PSwiftCharts()
//        PCustomLayoutView()
//        PNavigationSplitViewThreeColumn()
//        PNavigationSplitViewTwoColumn()
//        PNavigationStackDestination()
//        PNavigationStack()
//        PPasteButton()
//        PMultiDatePicker()
//        PRequestReview()
//        PShareLink()
//        PTapLocation()
//        PGrid()
//        PSheet()
    }
}



struct PTextTransitionsView: View {
    
    
    @State private var expandMessage = true
    private let mintWithShadow: AnyShapeStyle = AnyShapeStyle(Color.mint.shadow(.drop(radius: 2)))
    private let primaryWithoutShadow: AnyShapeStyle = AnyShapeStyle(Color.primary.shadow(.drop(radius: 0)))

    var body: some View {
        Text("Dai Ming Swift Pamphlet")
            .font(expandMessage ? .largeTitle.weight(.heavy) : .body)
            .foregroundStyle(expandMessage ? mintWithShadow : primaryWithoutShadow)
            .onTapGesture { withAnimation { expandMessage.toggle() }}
            .frame(maxWidth: expandMessage ? 150 : 250)
            .drawingGroup()
            .padding(20)
            .background(.cyan.opacity(0.3), in: RoundedRectangle(cornerRadius: 6))
    }
}

struct PButtonStyleComposition: View {
    @State private var isT = false
    var body: some View {
        Section("标签") {
            VStack(alignment: .leading) {
                HStack {
                    Toggle("Swift", isOn: $isT)
                    Toggle("SwiftUI", isOn: $isT)
                }
                HStack {
                    Toggle("Swift Chart", isOn: $isT)
                    Toggle("Navigation API", isOn: $isT)
                }
            }
            .toggleStyle(.button)
            .buttonStyle(.bordered)
        }
    }
}

struct PGradientAndShadow: View {
    var body: some View {
        Image(systemName: "bird")
            .frame(width: 150, height: 150)
            .background(in: Rectangle())
            .backgroundStyle(.cyan.gradient)
            .foregroundStyle(.white.shadow(.drop(radius: 1, y: 3.0)))
            .font(.system(size: 60))
    }
}

struct PGauge: View {
    @State private var progress = 0.45
    var body: some View {
        Gauge(value: progress) {
            Text("进度")
        } currentValueLabel: {
            Text(progress.formatted(.percent))
        } minimumValueLabel: {
            Text(0.formatted(.percent))
        } maximumValueLabel: {
            Text(100.formatted(.percent))
        }
        
        Gauge(value: progress) {
            
        } currentValueLabel: {
            Text(progress.formatted(.percent))
                .font(.footnote)
        }
        .gaugeStyle(.accessoryCircularCapacity)
        .tint(.cyan)
    }
}

struct PAnyLayout: View {
    @State private var isVertical = false
    var body: some View {
        let layout = isVertical ? AnyLayout(VStack()) : AnyLayout(HStack())
        layout {
            Image(systemName: "star").foregroundColor(.yellow)
            Text("Starming.com")
            Text("戴铭")
        }
        Button("Click") {
            withAnimation {
                isVertical.toggle()
            }
        } // end button
    } // end body
}

struct PViewThatFits: View {
    var body: some View {
        ViewThatFits(in: .horizontal) {
            HStack {
                Image(systemName: "heart")
                Text("收藏")
            }
            VStack {
                Image(systemName: "heart")
                Text("收藏")
            }
        }
    }
}

struct PSwiftCharts: View {
    struct CData: Identifiable {
        let id = UUID()
        let i: Int
        let v: Double
    }
    
    @State private var a: [CData] = [
        .init(i: 0, v: 2),
        .init(i: 1, v: 20),
        .init(i: 2, v: 3),
        .init(i: 3, v: 30),
        .init(i: 4, v: 8),
        .init(i: 5, v: 80)
    ]
    
    var body: some View {
        Chart(a) { i in
            LineMark(x: .value("Index", i.i), y: .value("Value", i.v))
            BarMark(x: .value("Index", i.i), yStart: .value("开始", 0), yEnd: .value("结束", i.v))
                .foregroundStyle(by: .value("Value", i.v))
        } // end Chart
    } // end body
}

struct PCustomLayout: Layout {
    
    private func maxSize(subviews: Subviews) -> CGSize {
        let subviewSizes = subviews.map { s in
            s.sizeThatFits(.unspecified)
        }
        let maxSize: CGSize = subviewSizes.reduce(.zero) { current, size in
            CGSize(width: max(current.width, size.width), height: max(current.height, size.height))
        }
        return maxSize
    }
    
    private func spacings(subviews: Subviews) -> [CGFloat] {
        let spacing: [CGFloat] = subviews.indices.map { i in
            guard i < subviews.count - 1 else {
                return 0.0
            }
            return subviews[i].spacing.distance(to: subviews[i + 1].spacing, along: .horizontal)
        }
        return spacing
    }
    
    func placeSubviews(in bounds: CGRect, proposal: ProposedViewSize, subviews: Subviews, cache: inout ()) {
        let maxSize = maxSize(subviews: subviews)
        let spacing = spacings(subviews: subviews)
        
        let size = ProposedViewSize(width: maxSize.width, height: maxSize.height)
        var x = bounds.minX + maxSize.width / 2
        for i in subviews.indices {
            subviews[i].place(at: CGPoint(x: x, y: bounds.minY), anchor: .center, proposal: size)
            x += maxSize.width + spacing[i]
        }
    }
    
    func sizeThatFits(proposal: ProposedViewSize, subviews: Subviews, cache: inout ()) -> CGSize {
        let maxSize = maxSize(subviews: subviews)
        let spacings = spacings(subviews: subviews)
        let totalSpacing = spacings.reduce(0.0, +)
        return CGSize(width: maxSize.width * CGFloat(subviews.count) + totalSpacing, height: maxSize.height)
    }
}

//struct PCustomLayoutView: View {
//    var body: some View {
//        PCustomLayout {
//            Text("Hello...")
//                            .foregroundColor(.red)
//                        Text("Hello.........")
//                            .foregroundColor(.green)
//                        Text("Hello..............")
//                            .foregroundColor(.blue)
//        }
//    }
//}



struct PNavigationSplitViewThreeColumn: View {
    struct Group: Identifiable, Hashable {
        let id = UUID()
        var title: String
        var subs: [String]
    }
    
    @State private var gps = [
        Group(title: "One", subs: ["o1", "o2", "o3"]),
        Group(title: "Two", subs: ["t1", "t2", "t3"])
    ]
    
    @State private var choiceGroup: Group?
    @State private var choiceSub: String?
    
    @State private var cv = NavigationSplitViewVisibility.automatic
    
    var body: some View {
        NavigationSplitView(columnVisibility: $cv) {
            List(gps, selection: $choiceGroup) { g in
                Text(g.title).tag(g)
            }
            .navigationSplitViewColumnWidth(250)
        } content: {
            List(choiceGroup?.subs ?? [], id: \.self, selection: $choiceSub) { s in
                Text(s)
            }
        } detail: {
            Text(choiceSub ?? "选一个")
            Button("点击") {
                cv = .all
            }
        }
        .navigationSplitViewStyle(.prominentDetail)
    }
}

struct PNavigationSplitViewTwoColumn: View {
    @State private var a = ["one", "two", "three"]
    @State private var choice: String?
    
    var body: some View {
        NavigationSplitView {
            List(a, id: \.self, selection: $choice, rowContent: Text.init)
        } detail: {
            Text(choice ?? "选一个")
        }
    }
}

struct PNavigationStackDestination: View {
    var body: some View {
        NavigationStack {
            List {
                NavigationLink(value: "字符串") {
                    Text("字符串")
                }
                NavigationLink(value: Color.red) {
                    Text("红色")
                }
            }
            .navigationTitle("不同类型 Destination")
            .navigationDestination(for: Color.self) { c in
                c.clipShape(Circle())
            }
            .navigationDestination(for: String.self) { s in
                Text("\(s) 的 detail")
            }
        }
    }
}

struct PNavigationStack: View {
    @State private var a = [1, 3, 9] // 深层链接
    var body: some View {
        NavigationStack(path: $a) {
            List(1..<10) { i in
                NavigationLink(value: i) {
                    Label("第 \(i) 行", systemImage: "\(i).circle")
                }
            }
            .navigationDestination(for: Int.self) { i in
                Text("第 \(i) 行内容")
            }
            .navigationTitle("NavigationStack Demo")
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
