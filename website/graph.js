var xyValues = [{ x: 0, y: 0 }];
function send_input_output() {
  var voltageInput = document.getElementById("voltage_input").value;
  var url = "https://api.thingspeak.com/update?api_key=GZO40J315M8AWC35&field1=";
  url = url.concat(voltageInput);
  var myWindow = window.open(url, "_blank");
  console.log(voltageInput);
  var start = new Date().getTime();

  setTimeout(() => {
    var xmlHttp = new XMLHttpRequest();
    // xmlHttp.open('GET', "https://api.thingspeak.com/channels/1904023/feeds.json?api_key=72Y6HM5F2PU9AZXC&results=1", true);
    xmlHttp.open('GET', "https://api.thingspeak.com/channels/1922371/feeds.json?api_key=9PK1URG5QE0ECH0F&results=1", true);
    xmlHttp.send();
    xmlHttp.onreadystatechange = function () {
      if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
        var input = JSON.parse(xmlHttp.responseText);
        console.log(input);
        rpmInput = input["feeds"][0]["field2"];
        console.log(rpmInput);
        xyValues.push({ x: voltageInput, y: rpmInput },);
        console.log(document.getElementById('rpmOutput').innerHTML);
        document.getElementById("rpmOutput").innerHTML = rpmInput;
      }
    }

    new Chart("myChart", {
      type: "scatter",
      data: {
        datasets: [{
          pointRadius: 4,
          pointBackgroundColor: "rgb(0,0,255)",
          data: xyValues
        }]
      },
      options: {
        legend: { display: false },
        scales: {
          xAxes: [{ ticks: { min: 0, max: 10 } }],
          yAxes: [{ ticks: { min: 0, max: 400 } }],
        }
      }
    });

    console.log(xyValues.length);
    myWindow.close();
  },5000);
}

function DcTheory() {
  window.open("theory_dcmotor.html", "_self");
}