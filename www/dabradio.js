
let currentService;
let ensemble;
let dabFreq;

//
// Get data
//
async function getRadioData() {

  initServiceInfo();

  let req = await fetch("/current", {cache: "no-store"});
  currentService = await req.json();

  req = await fetch("/freq", {cache: "no-store"});
  dabFreq = await req.json();

  req = await fetch("/ensemble", {cache: "no-store"});
  ensemble = await req.json();

  populateServiceInfo();
  populateChannelSelector();
  populateServiceSelector();

}

function initServiceInfo() {
  let myP = document.getElementById('serviceName');
  myP.textContent = "Loading...";

  myP = document.getElementById('channelBlock');
  myP.textContent = "Block: ??";

  myP = document.getElementById('channelFreq');
  myP.textContent = "Frequency: ??";

  myP = document.getElementById('ensembleName');
  myP.textContent = "Ensemble: ??";

  myP = document.getElementById('signalQuality');
  myP.textContent = "RSSI: ?? dBuV  SNR: ?? dB  CNR: ?? dB  FIC quality: ?? %"; 

}

//
// Store current service information
//
function populateServiceInfo() {

  let myP = document.getElementById('serviceName');
  myP.textContent = currentService.serviceLabel;

  myP = document.getElementById('channelBlock');
  myP.textContent = "Block: " + currentService.block;

  myP = document.getElementById('channelFreq');
  myP.textContent = "Frequency: " + currentService.freq;

  myP = document.getElementById('ensembleName');
  myP.textContent = "Ensemble: " + currentService.ensembleName;

  myP = document.getElementById('signalQuality');
  myP.textContent = "RSSI: " + currentService.rssi + "  ";
  myP.textContent = myP.textContent + "SNR: " + currentService.snr + "  ";
  myP.textContent = myP.textContent + "CNR: " + currentService.cnr + "  ";
  myP.textContent = myP.textContent + "FIC quality: " + currentService.ficQuality;

}

//
// Load up channel selector
//
function populateChannelSelector() {

  const chSel= document.getElementById('channelSelector');
  removeOptions(chSel);

  let opt;
  for(let i=0; i < dabFreq.numFreq; i++) {
    opt = document.createElement("option");
    opt.text = dabFreq.freqs[i].block + "  (" + dabFreq.freqs[i].freq + ")";
    if(dabFreq.freqs[i].block == currentService.block) {
      opt.selected=true;
    }
    chSel.options.add(opt, -1);
  }
}

//
// Load up programme labels into serviceSelector
//
function populateServiceSelector() {

  stopPlayer();

  const srvSel= document.getElementById('serviceSelector');
  removeOptions(srvSel);

  let opt;
  for(let i=0; i < ensemble.numServices; i++) {
    opt = document.createElement("option");
    opt.text = ensemble.services[i].label;
    if(ensemble.services[i].label == currentService.serviceLabel) {
      opt.selected=true;
    }
    srvSel.options.add(opt, -1);
  }
}

function removeOptions(selectElement) {
   var i, L = selectElement.options.length - 1;
   for(i = L; i >= 0; i--) {
      selectElement.remove(i);
   }
}

function stopPlayer() {
  const player = document.getElementById('player');
  player.pause();
}

window.onload = function() {
  channelSelector.onchange = async function() {
    stopPlayer();
    initServiceInfo();

    const url = "/channel&"+channelSelector.selectedIndex;
    console.log(url);
    let req = await fetch(url, {cache: "no-store"});
    currentService = await req.json();

    req = await fetch("/ensemble", {cache: "no-store"});
    ensemble = await req.json();

    console.log("Got response");

    populateServiceInfo();
    populateServiceSelector();
  }

  serviceSelector.onchange = async function() {
    stopPlayer();
    initServiceInfo();

    const srv = serviceSelector.selectedIndex;
    let url = "/service&" + ensemble.services[srv].servid;
    url = url + "&" + ensemble.services[srv].compid;
    
    console.log(url);
    let req = await fetch(url, {cache: "no-store"});
    currentService = await req.json();

    console.log("Got response");

    populateServiceInfo();
  }

  getRadioData();
}

