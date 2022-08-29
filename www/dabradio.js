
let currentService;
let ensemble;
let dabFreq;

//
// Get data
//
async function getRadioData() {

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

window.onload = function() {
  channelSelector.onchange = function() {
    console.log(channelSelector.value);
    let url = "/channel&freq="+channelSelector.selectedIndex;
//    let req = await fetch(url, {cache: "no-store"});
    console.log(url);
  }

  serviceSelector.onchange = function() {
//  let req = await fetch("/current", {cache: "no-store"});
  }

  getRadioData();
}

