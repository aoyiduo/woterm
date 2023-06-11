import QtQml 2.2
import QtQuick 2.12
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import QtCharts 2.3

Rectangle {
    id: thiz
    color: Playbook.backgroundColor
    property int chartWidth: 600
    property int chartHeight: 500

    Timer {
        property int idx: 0
        interval: 1000*2
        repeat: true
        running: true
        onTriggered: {
            pie.update()
            line.update()
            bar.update()
            percent.update()
        }
    }

    ScrollView {
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded
        contentItem: Flow {
            width:  Math.floor(thiz.width / chartWidth) * chartWidth
            ChartView {
                id: pie
                width: chartWidth
                height: chartHeight
                title: "Language"
                theme: ChartView.ChartThemeBrownSand
                antialiasing: true

                PieSeries {                    
                    id: pieSeries

                    //PieSlice { label: "eaten"; value: 94.9 }
                    //PieSlice { label: "not yet eaten"; value: 5.1 }

                }
                function update() {
                    let zhv = Math.floor(Math.random() * 100);
                    let env = 100 - zhv;
                    let pzh = pieSeries.find("ZH")
                    if(pzh) {
                        pzh.value = zhv;
                    }else{
                        pieSeries.append("ZH", zhv)
                    }
                    let pen = pieSeries.find("EN")
                    if(pen) {
                        pen.value = env;
                    }else{
                        pieSeries.append("EN", env)
                    }
                }
            }

            ChartView {
                id: line
                width: chartWidth
                height: chartHeight
                theme: ChartView.ChartThemeBrownSand
                title: "CPU"
                antialiasing: true

                ValueAxis {
                    id: line_X
                    min: 0
                    max: 10
                    tickCount: 1
                }

                ValueAxis {
                    id: line_Y
                    min: 0
                    max: 10
                    tickCount: 1
                }

                LineSeries {
                    id: host1
                    name: "Host1"
                    axisX: line_X
                    axisY: line_Y
                    color: "blue"
                    //XYPoint { x: 0; y: 0 }
                    //XYPoint { x: 1.1; y: 2.1 }
                    //XYPoint { x: 1.9; y: 3.3 }
                }
                LineSeries {
                    id: host2
                    name: "Host2"
                    color: "green"
                    //XYPoint { x: 0; y: 0 }
                    //XYPoint { x: 1.1; y: 2.1 }
                    //XYPoint { x: 1.9; y: 3.3 }
                }

                property int istep: 1

                function update() {
                    host1.clear()
                    host1.append(0, 0)
                    istep++;
                    line_X.min = istep
                    line_X.max = istep+10
                    for(let i = 1; i < 10; i++) {
                        host1.append(istep+i, Math.floor(Math.random() * 10))
                    }
                    host2.clear()
                    host2.append(0, 0)
                    for(let j = 1; j < 10; j++) {
                        host2.append(istep+j, Math.floor(Math.random() * 10))
                    }
                }
            }

            ChartView {
                id: bar
                width: chartWidth
                height: chartHeight
                theme: ChartView.ChartThemeBrownSand
                title: "DayUsers"
                antialiasing: true

                ValueAxis {
                    id: bar_Y
                    min: 0
                    max: 20
                    tickCount: 1
                }

                BarSeries {
                    id: barSeries
                    axisX: BarCategoryAxis { categories: ["Day1", "Day2", "Day3", "Day4", "Day5", "Day6", "Day7"] }
                    axisY: bar_Y
                    BarSet { label: "User"; values: [2, 2, 3, 4, 5, 6, 7] }
                }

                function update() {
                    barSeries.clear()
                    let vals = []
                    for(let i = 0; i < 7; i++) {
                        vals.push(Math.floor(Math.random() * 10 + 5))
                    }
                    barSeries.append("User", vals)
                }
            }

            ChartView {
                id: percent
                width: chartWidth
                height: chartHeight
                theme: ChartView.ChartThemeBrownSand
                title: "Percent"
                antialiasing: true

                PercentBarSeries {
                    axisX: BarCategoryAxis { categories: ["2007", "2008", "2009", "2010", "2011", "2012" ] }
                    BarSet { id: bob; label: "Bob"; values: [2, 2, 3, 4, 5, 6] }
                    BarSet { id: susan; label: "Susan"; values: [5, 1, 2, 4, 1, 7] }
                    BarSet { id: james; label: "James"; values: [3, 5, 8, 13, 5, 8] }
                }

                function update() {
                    let i = 0;
                    let vals = [];
                    for(i = 0; i < 6; i++) {
                        vals.push(Math.floor(Math.random() * 5 + 1));
                    }
                    bob.values = vals
                    vals = []
                    for(i = 0; i < 6; i++) {
                        vals.push(Math.floor(Math.random() * 5 + 1));
                    }
                    susan.values = vals
                    vals = []
                    for(i = 0; i < 6; i++) {
                        vals.push(Math.floor(Math.random() * 5 + 1));
                    }
                    james.values = vals
                }

            }

            ChartView {
                width: chartWidth
                height: chartHeight
                theme: ChartView.ChartThemeBrownSand
                title: "bar"
                antialiasing: true

                BarSeries {
                    axisX: BarCategoryAxis { categories: ["2007", "2008", "2009", "2010", "2011", "2012" ] }
                    BarSet { label: "Bob"; values: [2, 2, 3, 4, 5, 6] }
                    BarSet { label: "Susan"; values: [5, 1, 2, 4, 1, 7] }
                    BarSet { label: "James"; values: [3, 5, 8, 13, 5, 8] }
                }
            }

        }
    }
}
