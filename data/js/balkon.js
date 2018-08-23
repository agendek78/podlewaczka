function createChart(obj, name) {
    let chart = new Chart(obj, {
        type: 'line',
        data: {
            datasets: [{
                lineTension: 0,
                backgroundColor: 'transparent',
                borderColor: '#007bff',
                borderWidth: 4,
                pointBackgroundColor: '#007bff'
            }]
        },
        options: {
            scales: {
                yAxes: [{
                    ticks: {
                        beginAtZero: false
                    }
                }]
            },
            legend: {
                display: false,
            },
            title: {
                display: true,
                text: name
            },
            elements: {
                line: {
                    cubicInterpolationMode: 'default'
                }
            },
            layout: {
                padding: {
                    left: 20,
                    right: 0,
                    top: 0,
                    bottom: 0
                }
            }
        }
    });
    return chart;
}

var tankChart = new Chart(document.getElementById("tankChart"), {
    type: 'bar',
    data: {
        labels: ["Zbiornik 1", "Zbiornik 2", "Zbiornik 3"],
        datasets: [{
            borderWidth: 2
        }]
    },
    options: {
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero: true
                }
            }]
        },
        legend: {
            display: false,
        },
        title: {
            display: true,
            text: 'Stan zbiorników na wodę'
        },
        layout: {
            padding: {
                left: 20,
                right: 0,
                top: 0,
                bottom: 0
            }
        }
    }
});

var tempDayChart = createChart(document.getElementById("dayTempChart"), "Temperatura dziś");
var tempMonthChart = createChart(document.getElementById("monthTempChart"), "Temperatura w miesiącu");
var pressDayChart = createChart(document.getElementById("dayPressChart"), "Ciśnienie dziś");
var pressMonthChart = createChart(document.getElementById("monthPressChart"), "Ciśnienie w miesiącu");
var humDayChart = createChart(document.getElementById("dayHumChart"), "Wilgotność dziś");
var humMonthChart = createChart(document.getElementById("monthHumChart"), "Wilgotność w miesiącu");
var luxDayChart = createChart(document.getElementById("dayLuxChart"), "Naświetlenie dziś");
var luxMonthChart = createChart(document.getElementById("monthLuxChart"), "Naświetlenie w miesiącu");

var activeTab = '#temperature';
var lastMonthPressTime = new Date(0);
var refreshTimerInterval = 60000;
var timer;

function parseDataJSON(json, day)
{
    let newData = [];
    let newX = [];

    for ( var i = 0; i < json.ts.length; i++ ) 
    {
        var d = new Date(json.ts[i] * 1000);
        if (day == true)
        {            
            newX.push(d.format("HH:MM", true));
        }
        else
        {
            newX.push(d.format("dd.mm", true));
        }
    }

    for ( var i = 0; i < json.vals.length; i++ ) {
        newData.push(json.vals[i]);
    }

    return { data: newData, labels: newX};
}

function onDayMeasJSON(json)
{
    var data = parseDataJSON(json, true);

    if (activeTab == '#temperature')
    {
        $('#current-temp-value').html(json.curr + " °C");
        tempDayChart.data.labels = data.labels;
        tempDayChart.data.datasets[0].data = data.data;
        tempDayChart.update();        
    }
    else if (activeTab == '#pressure')
    {
        $('#current-pressure-value').html(json.curr + " hPa");
        pressDayChart.data.labels = data.labels;
        pressDayChart.data.datasets[0].data = data.data;
        pressDayChart.update();
    }
    else if (activeTab == '#humidity')
    {
        $('#current-hum-value').html(json.curr + " %");
        humDayChart.data.labels = data.labels;
        humDayChart.data.datasets[0].data = data.data;
        humDayChart.update();
    }    
    else if (activeTab == '#luxlevel')
    {
        $('#current-lux-value').html(json.curr + " lux");
        luxDayChart.data.labels = data.labels;
        luxDayChart.data.datasets[0].data = data.data;
        luxDayChart.update();
    }
}

var goodLevelBgCol ='rgba(75, 192, 100, 0.4)';
var mediumLevelBgCol = 'rgba(255, 205, 86, 0.4)';
var lowLevelBgCol = 'rgba(255, 99, 132, 0.4)';
var goodLevelLCol ='rgb(75, 192, 100)';
var mediumLevelLCol = 'rgb(255, 205, 86)';
var lowLevelLCol = 'rgb(255, 99, 132)';

function onIrrigationJSON(json)
{
    var tbody = document.getElementById("podlewanie-dane");

    for(var i = 0; i < tbody.childElementCount; i++)
    {
        var r = tbody.childNodes[i];
        if (i < json.g.length)
        {            
            r.childNodes[0].innerText = json.g[i].t;
            if (json.g[i].st == 0)
            {
                r.childNodes[1].childNodes[0].attributes.getNamedItem("class").value = "d-none";
                r.childNodes[1].childNodes[1].attributes.getNamedItem("class").value = "";                
                r.childNodes[1].childNodes[2].attributes.getNamedItem("class").value = "d-none";
            } else if (json.g[i].st == 1)
            {
                r.childNodes[1].childNodes[0].attributes.getNamedItem("class").value = "";
                r.childNodes[1].childNodes[1].attributes.getNamedItem("class").value = "d-none";                
                r.childNodes[1].childNodes[2].attributes.getNamedItem("class").value = "d-none";
            } else if (json.g[i].st == 2)
            {
                r.childNodes[1].childNodes[0].attributes.getNamedItem("class").value = "d-none";
                r.childNodes[1].childNodes[1].attributes.getNamedItem("class").value = "d-none";                
                r.childNodes[1].childNodes[2].attributes.getNamedItem("class").value = "";
            }

            for(var j = 0; j < json.g[i].en.length; j++)
            {
                r.querySelector("#enCh"+(j+1)).checked = (json.g[i].en[j] != 0);
            }

            r.attributes.getNamedItem("class").value = "";
        }
        else
        {
            r.attributes.getNamedItem("class").value = "d-none";
        }
    }
    
    var tankLevels = [];
    var tankBgCols = [];
    var tankLCols = [];

    for(var i = 0; i < json.ts.length; i++)
    {
        tankLevels.push(json.ts[i]);
        if (json.ts[i] < 60)
        {
            tankBgCols.push(mediumLevelBgCol);
            tankLCols.push(mediumLevelLCol);
        } else if (json.ts[i] < 30)
        {
            tankBgCols.push(lowLevelBgCol);
            tankLCols.push(lowLevelLCol);
        } else
        {
            tankBgCols.push(goodLevelBgCol);
            tankLCols.push(goodLevelLCol);
        }
    }
    tankChart.data.datasets[0].data = tankLevels;
    tankChart.data.datasets[0].backgroundColor = tankBgCols;
    tankChart.data.datasets[0].borderColor = tankLCols;
    tankChart.update();
}

function updateGraph(){
    
    if (activeTab == '#temperature')
    {
        $.getJSON('/temp.json', onDayMeasJSON);
    }
    else if (activeTab == '#pressure')
    {
        $.getJSON('/pressure.json', onDayMeasJSON);
    }
    else if (activeTab == '#humidity')
    {
        $.getJSON('/humidity.json', onDayMeasJSON);
    }
    else if (activeTab == '#luxlevel')
    {
        $.getJSON('/lux.json', onDayMeasJSON);
    }
    else if (activeTab == '#irrigation')
    {
        $.getJSON('/podlewanie.json', onIrrigationJSON);
    }
}

function loadMonthData()
{
    if (activeTab == '#temperature' && tempMonthChart.data.datasets[0].data.length == 0)
    {
        $.getJSON('/tempMonth.json', function(json){
            var data = parseDataJSON(json, false);
            tempMonthChart.data.labels = data.labels;
            tempMonthChart.data.datasets[0].data = data.data;
            tempMonthChart.update();
        });
    }
    else if (activeTab == '#pressure' && pressMonthChart.data.datasets[0].data.length == 0)
    {
        $.getJSON('/pressMonth.json', function(json){
            var data = parseDataJSON(json, false);
            pressMonthChart.data.labels = data.labels;
            pressMonthChart.data.datasets[0].data = data.data;
            pressMonthChart.update();
        });
    }
    else if (activeTab == '#humidity' && humMonthChart.data.datasets[0].data.length == 0)
    {
        $.getJSON('/humMonth.json', function(json){
            var data = parseDataJSON(json, false);
            humMonthChart.data.labels = data.labels;
            humMonthChart.data.datasets[0].data = data.data;
            humMonthChart.update();
        });
    }
    else if (activeTab == '#luxlevel' && luxMonthChart.data.datasets[0].data.length == 0)
    {
        $.getJSON('/humMonth.json', function(json){
            var data = parseDataJSON(json, false);
            luxMonthChart.data.labels = data.labels;
            luxMonthChart.data.datasets[0].data = data.data;
            luxMonthChart.update();
        });
    }

}

timer = setInterval(updateGraph, refreshTimerInterval);
updateGraph();
loadMonthData();

$('#pills-tab').on('shown.bs.tab', function (e) {
    tab_pane = $(e.target).attr("href")  
    //console.log('activated ' + tab_pane );  
    activeTab = tab_pane;
    loadMonthData();
    clearInterval(timer);
    updateGraph();
    timer = setInterval(updateGraph, refreshTimerInterval);
  });

$('#settings-submit').on('click', function(ev){

    var tbody = document.getElementById("podlewanie-dane");
    var reqStr = '';

    for(var i = 0; i < tbody.childElementCount; i++)
    {
        var r = tbody.childNodes[i];

        for(var j = 0; j < r.childElementCount; j++)
        {
            var val = 0;

            if (r.querySelector("#enCh"+(j+1)).checked)
            {
                val = 1;
            }

            if (reqStr.length > 0)
            {
                reqStr += "&";
            }
            reqStr += "r"+i+"c"+j+"="+val;
        }
    }
    $.post("sett?"+reqStr);
}); 