const { app, BrowserWindow, crashReporter } = require("electron")
const electron = require('electron')
const path = require('path')
const fs = require("fs")

const windows_to_test = 10;

let test_started = false;
let windows = [];
let windows_created = 0;

crashReporter.start({uploadToServer: false, submitURL: '', ignoreSystemCrashHandler : true});

function startCreatingWindows() {
    createWindow();
}

function createWindow() {
    test_started = true;
    console.log('--------------- step create window ' + windows_created);
    console.log('');

    let win = new BrowserWindow({
        x: Math.floor(Math.random() * 1000), 
        y: Math.floor(Math.random() * 700),
        webPreferences: {

            preload: path.join(__dirname, 'preload_win.js'),
            nodeIntegration: true,
        },
    });
    win.loadFile('index.html')

    win.on("closed", () => { windows.emowin = null });

    windows.unshift(win);
    windows_created = windows_created + 1;

    if (windows_created < windows_to_test) {
        setTimeout(createWindow, 1000);
    }

    setTimeout(closeWindow, 3000);
}

app.on("ready", startCreatingWindows);

app.on("window-all-closed", () => {
    app.quit();
})

app.on("render-process-gone", (e, webContents, details) => {
    console.log('--------------- get event: render-process-gone ', details);
    app.quit();
  });

function closeWindow() {
    console.log('--------------- step close window , windows left : ' + windows.length);
    console.log('');

    let win = windows.pop();
    win.close();

    if (windows.length == 0) {
        setTimeout(step_finish, 500);
    }
 
}

function step_finish() {
    crashReporter.getLastCrashReport().then((report) => { console.log(report); });

    app.quit();
}