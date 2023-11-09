var x_graph = [];
var y_graph_coffee = [];
var y_graph_music = [];
var y_graph_light = [];
var y_graph_backup = [];

var array_length = 30;
for (let i = 0; i < array_length; i++) {
    x_graph[i] = i;
}

const myChart = new Chart("myChart", {
    type: "line",
    data: {
        labels: x_graph,
        datasets: [
        {
            fill: false,
            lineTension: 0,
            data: y_graph_coffee,
            backgroundColor: "#00b0a0",
            borderColor: "#00b0a0",
        },
        {
            fill: false,
            lineTension: 0,
            data: y_graph_light,
            backgroundColor: "#fff700C0",
            borderColor: "#fff700",
        },
        {
            fill: false,
            lineTension: 0.5,
            data: y_graph_music,
            backgroundColor: "#A000ffC0",
            borderColor: "#A000ff",
        },
        {
            fill: false,
            lineTension: 0,
            data: y_graph_backup,
            backgroundColor: "#ff6000",
            borderColor: "#ff6000",
        }],
    },
    options: {
        legend: { display: false },  // Set to true to differentiate the datasets
        scales: {
            yAxes: [{ ticks: { min: 0, max: 100 } ,
                display: false,
                scaleLabel: {
                    display: true,
                    labelString: '%',
                    fontSize: 20,
                }}],
                xAxes: [{
                    ticks: {
                        min: -1,
                        max: 30
                    },
                    display: false,
                    scaleLabel: {
                        display: true,
                        labelString: 'Time (minutes)'
                    }
                }]
        },
    },
});


function update_graph_coffee(data) {
    console.log(data);
    myChart.data.datasets[0].data = data;
    myChart.update();
}
    
function update_graph_light(data) {
    console.log(data);
    myChart.data.datasets[1].data = data;
    myChart.update();
}

function update_graph_music(data) {
    console.log(data);
    myChart.data.datasets[2].data = data;
    myChart.update();
}

function update_graph_backup(data) {
    console.log(data);
    myChart.data.datasets[3].data = data;
    myChart.update();
}
    