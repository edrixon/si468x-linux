
let currentService;
let ensemble;
let dabFreq;
let sysInfo;
let serviceDataLastMs;
let serviceData;
let intervalId;
let audioSrc;
let isPlaying;
let audioUrl;

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

  req = await fetch("/system", {cache: "no-store"});
  sysInfo = await req.json();

  populateServiceInfo();
  populateChannelSelector();
  populateServiceSelector();
  populateSysInfo();
}

function playerLoadStart() {
  const myP = document.getElementById('progress');
  myP.textContent = "Loading...";
}

function playerLoadedData() {
  const myP = document.getElementById('progress');
  myP.textContent = "Buffering...";
}

function playerProgress() {
  const myP = document.getElementById('progress');
  myP.textContent = "Playing...";
}

function playerCanPlay() {
  const myP = document.getElementById('progress');
  myP.textContent = "";
}

function initPlayer() {
  const button = document.getElementById('play');
  button.src = "speaker.jpg";

  const myP = document.getElementById('progress');
  myP.textContent = "";

  const player = document.getElementById('player');
  player.setAttribute('preload', 'none');
  player.addEventListener("canplay", playerCanPlay);
  player.addEventListener("loadstart", playerLoadStart);
  player.addEventListener("loadeddata", playerLoadedData);
  player.addEventListener("progress", playerProgress);

  isPlaying = false;

  audioUrl = "http://dabrx.ednet.pri:8000/dab.mp3";

}

function initServiceInfo() {
  let myP = document.getElementById('serviceName');
  myP.textContent = "Loading...";

  myP = document.getElementById('dabTime');
  myP.textContent = "??";

  myP = document.getElementById('channelBlock');
  myP.textContent = "Block: ??";

  myP = document.getElementById('channelFreq');
  myP.textContent = "Frequency: ??";

  myP = document.getElementById('ensembleName');
  myP.textContent = "Ensemble: ??";

  myP = document.getElementById('channelInfo');
  myP.textContent = "Bit rate: ??  Mode: ??  Service: ??  Protection: ??";

  myP = document.getElementById('signalQuality');
  myP.textContent = "RSSI: ?? dBuV  SNR: ?? dB  CNR: ?? dB  FIC quality: ?? %"; 

  myP = document.getElementById('serviceData');
  myP.textContent = "";
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

  myP = document.getElementById('channelInfo');
  myP.textContent = "Bit rate: " + currentService.bitRate;
  myP.textContent = myP.textContent + "  Mode: " + currentService.mode;
  myP.textContent = myP.textContent + "  Service: "
                                                  + currentService.serviceMode;
  myP.textContent = myP.textContent + "  Protection: "
                                               + currentService.protectionInfo;

  myP = document.getElementById('ensembleName');
  myP.textContent = "Ensemble: " + currentService.ensembleName;

  populateSignalQuality();
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

function populateSysInfo() {
  let myP = document.getElementById('sysinfo');
  myP.textContent = "Hw: " + sysInfo.partNo;
  myP.textContent = myP.textContent + " Sw: " + sysInfo.swVer;
}

function removeOptions(selectElement) {
   var i, L = selectElement.options.length - 1;
   for(i = L; i >= 0; i--) {
      selectElement.remove(i);
   }
}

function stopPlayer() {

  isPlaying = false;

  const button = document.getElementById('play');
  button.src = "speaker.jpg";

  const myP = document.getElementById('progress');
  myP.textContent = "";

  const player = document.getElementById('player');
  player.pause();
  player.removeAttribute('src');
  player.load();

}

function startPlayer() {

  isPlaying = true;

  const button = document.getElementById('play');
  button.src = "mute.jpg";

  const player = document.getElementById('player');
  player.setAttribute('src', audioUrl);
  player.play();

}

function populateSignalQuality() {

  const myP = document.getElementById('signalQuality');
  myP.textContent = "RSSI: " + currentService.rssi + "  ";
  myP.textContent = myP.textContent + "SNR: " + currentService.snr + "  ";
  myP.textContent = myP.textContent + "CNR: " + currentService.cnr + "  ";
  myP.textContent = myP.textContent + "FIC quality: " + currentService.ficQuality;

}

async function servInfo() {

  let req = await fetch("/servicedata", {cache: "no-store"});
  serviceData = await req.json();

  req = await fetch("/current", {cache: "no-store"});
  currentService = await req.json();

  if(serviceData.serviceDataMs != serviceDataLastMs) {
    serviceDataLastMs = serviceData.serviceDataMs;

    const myP = document.getElementById('serviceData');
    myP.textContent = serviceData.serviceData;
  }

  populateSignalQuality();

  myP = document.getElementById('dabTime');
  myP.textContent = currentService.time;
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

    populateServiceInfo();
  }

  play.onclick = function() {
    if(isPlaying == false) {
      startPlayer();
    } else {
      stopPlayer();
    }
  }

  getRadioData();

  initPlayer();

  serviceDataLastMs = 0;
  intervalId = setInterval(servInfo, 20000);
}
