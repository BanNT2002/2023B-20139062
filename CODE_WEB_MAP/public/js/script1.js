// Cấu hình Firebase (thay thế bằng cấu hình Firebase thực tế của bạn)
const firebaseConfig = {
  apiKey: "AIzaSyB6M4zLSkDBal7azySrO957DoDIL0DmDoM",
  authDomain: "position-ble.firebaseapp.com",
  databaseURL: "https://position-ble-default-rtdb.firebaseio.com",
  projectId: "position-ble",
  storageBucket: "position-ble.appspot.com",
  messagingSenderId: "74836503453",
  appId: "1:74836503453:web:65513f97919315c00c1d74",
  measurementId: "G-EY3JLM9GY9",
};

firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// Array of beacon tags with their corresponding positions and colors
const beaconTagForm = document.getElementById("beacontag-form");
const beacontagRef = database.ref("/position");
let highestBeaconTagNumber = 0;
const container = document.querySelector(".bg-container");
const dots = {};

// Common function to draw a dot on the map
function drawDot(px, py, name, type) {
  const imgElement = document.querySelector(".bg");
  const imgWidth = imgElement.naturalWidth;
  const imgHeight = imgElement.naturalHeight;
  const x = (px / imgWidth) * 100;
  const y = (py / imgHeight) * 100;
  const a = x * 10;
  const b = y * 10;

  let dot = dots[name];
  if (!dot) {
    dot = document.createElement("div");
    dot.className = `dot ${type}`;
    if (type === "beacontag") {
      dot.className = "fa-solid fa-person-rays dot beacontag";
    } else {
      dot.className = "fa-solid fa-crosshairs dot ibeacon ";
    }
    dot.style.left = `${a}%`;
    dot.style.top = `${b}%`;

    const label = document.createElement("div");
    label.className = "dot-label";
    label.innerHTML = name + "<br>(" + px + ", " + py + ")";
    dot.appendChild(label);

    container.appendChild(dot);
    dots[name] = dot;
  } else {
    dot.style.left = `${a}%`;
    dot.style.top = `${b}%`;
    const label = dot.querySelector(".dot-label");
    label.innerHTML = name + "<br>(" + px + ", " + py + ")";
  }
}

// Handle form submission for beacon tags
beaconTagForm.addEventListener("submit", function (event) {
  event.preventDefault();

  const pxInput = document.getElementById("px");
  const pyInput = document.getElementById("py");

  const px = parseFloat(pxInput.value);
  const py = parseFloat(pyInput.value);
  console.log(px, py);
  if (isNaN(px) || isNaN(py)) {
    alert("Invalid input. Please enter valid numbers for Px and Py.");
    return;
  }

  if (px >= 82.5 || px < 0 || py >= 58.5 || py < 0) {
    alert(
      "Invalid input. Please enter valid numbers for Px and Py within the specified range."
    );
    return;
  }

  const beaconTagName = `beacontag${highestBeaconTagNumber}`;
  highestBeaconTagNumber++;

  database.ref(`position/${beaconTagName}`).set({ Px: px, Py: py });
  alert(`Data updated to Firebase for ${beaconTagName}!`);

  drawDot(px, py, beaconTagName, "beacontag");

  pxInput.value = "";
  pyInput.value = "";
});

// Load existing beacon tags and set up listeners
window.addEventListener("load", function () {
  beacontagRef.once("value", (snapshot) => {
    snapshot.forEach((childSnapshot) => {
      const beaconTagNumber = parseInt(
        childSnapshot.key.replace("beacontag", "")
      );
      if (
        !isNaN(beaconTagNumber) &&
        beaconTagNumber >= highestBeaconTagNumber
      ) {
        highestBeaconTagNumber = beaconTagNumber + 1;
      }
    });
  });

  beacontagRef.on("child_changed", (snapshot) => {
    const { Px: px, Py: py } = snapshot.val();
    drawDot(px, py, snapshot.key, "beacontag");
  });

  beacontagRef.on("child_added", (snapshot) => {
    const { Px: px, Py: py } = snapshot.val();
    drawDot(px, py, snapshot.key, "beacontag");
  });

  beacontagRef.on("child_removed", (snapshot) => {
    const dot = dots[snapshot.key];
    if (dot) {
      dot.remove();
      delete dots[snapshot.key];
    }
  });
});
//

const coordinatesRef = database.ref("/beacons");
let highestBeaconNumber = 0;

// Save coordinates to Firebase
function saveCoordinates(px, py, beaconName) {
  database.ref(`beacons/${beaconName}`).set({ Px: px, Py: py });
  alert(`Data updated to Firebase for ${beaconName}!`);
}

// Handle form submission for ibeacons
document.getElementById("my-form").addEventListener("submit", function (event) {
  event.preventDefault();

  const px = parseFloat(document.getElementById("px-input").value);
  const py = parseFloat(document.getElementById("py-input").value);

  if (isNaN(px) || isNaN(py)) {
    alert("Invalid input. Please enter valid numbers for Px and Py.");
    return;
  }

  if (px >= 82.5 || px < 0 || py >= 58.5 || py < 0) {
    alert(
      "Invalid input. Please enter valid numbers for Px and Py within the specified range."
    );
    return;
  }

  const beaconName = `ibeacon${highestBeaconNumber + 1}`;
  highestBeaconNumber++;
  saveCoordinates(px, py, beaconName);
  drawDot(px, py, beaconName, "ibeacon");
});

// Load existing beacons and set up listeners
window.addEventListener("load", function () {
  coordinatesRef.once("value", (snapshot) => {
    snapshot.forEach((childSnapshot) => {
      const beaconIndex = parseInt(childSnapshot.key.replace("ibeacon", ""));
      highestBeaconNumber = Math.max(highestBeaconNumber, beaconIndex);

      const { Px: px, Py: py } = childSnapshot.val();
      const beaconName = `ibeacon${beaconIndex}`;
      drawDot(px, py, beaconName, "ibeacon");
    });
  });

  coordinatesRef.on("child_changed", (snapshot) => {
    const { Px: px, Py: py } = snapshot.val();
    drawDot(px, py, snapshot.key, "ibeacon");
  });

  coordinatesRef.on("child_added", (snapshot) => {
    const { Px: px, Py: py } = snapshot.val();
    drawDot(px, py, snapshot.key, "ibeacon");
  });

  coordinatesRef.on("child_removed", (snapshot) => {
    const dot = dots[snapshot.key];
    if (dot) {
      dot.remove();
      delete dots[snapshot.key];
    }
  });
});

// Handle beacon form submission
document
  .getElementById("beacon-form")
  .addEventListener("submit", function (event) {
    event.preventDefault();

    const beaconName = document.getElementById("beacon-name").value;
    const distance = parseFloat(document.getElementById("distance").value);
    const axis = document.getElementById("axis").value;

    // Retrieve the specified beacon's current coordinates
    coordinatesRef.child(beaconName).once("value", function (snapshot) {
      if (snapshot.exists()) {
        const data = snapshot.val();
        let { Px: px, Py: py } = data;

        // Update coordinates based on the specified axis and distance
        if (axis === "x") {
          px += distance;
        } else if (axis === "y") {
          py += distance;
        }

        // Validate new coordinates
        if (px > 82.5 || py > 58.5) {
          alert(
            "The new coordinates exceed the allowed limits (Px <= 48, Py <= 62). Please enter a valid distance."
          );
          return;
        }

        // Check if a beacon with the new coordinates already exists
        coordinatesRef.once("value", function (snapshot) {
          let beaconExists = false;
          snapshot.forEach(function (childSnapshot) {
            const { Px: existingPx, Py: existingPy } = childSnapshot.val();
            if (existingPx === px && existingPy === py) {
              beaconExists = true;
              alert(
                "A beacon with these coordinates already exists. Please enter a different distance."
              );
              return;
            }
          });

          // Save the new beacon if no conflict exists
          if (!beaconExists) {
            const newBeaconName = `ibeacon${highestBeaconNumber + 1}`;
            highestBeaconNumber++;
            saveCoordinates(px, py, newBeaconName);
            drawDot(px, py, newBeaconName);
          }
        });
      } else {
        alert("iBeacon name does not exist.");
      }
    });
  });

// Handle beacon deletion form submission
document
  .getElementById("delete-form")
  .addEventListener("submit", function (event) {
    event.preventDefault();

    const beaconName = document.getElementById("delete-beacon-name").value;
    const beaconTagName = document.getElementById("delete-beacon-name").value;

    // Check if the specified beacon exists
    coordinatesRef.child(beaconName).once("value", function (snapshot) {
      if (snapshot.exists()) {
        // Remove the beacon from Firebase
        coordinatesRef.child(beaconName).remove();

        // Remove the dot from the DOM
        const dot = dots[beaconName];
        if (dot) {
          dot.remove();
          delete dots[beaconName];
        }

        alert(`${beaconName} has been deleted!`);
      } else {
      }
    });
    //
    beacontagRef.child(beaconTagName).once("value", function (snapshot) {
      if (snapshot.exists()) {
        // Remove the beacon from Firebase
        beacontagRef.child(beaconTagName).remove();

        // Remove the dot from the DOM
        const dot = dots[beaconTagName];
        if (dot) {
          dot.remove();
          delete dots[beaconTagName];
        }

        alert(`${beaconTagName} has been deleted!`);
      } else {
      }
    });
  });

// Tính khoảng cách giữa hai điểm
function calculateDistance(x1, y1, x2, y2) {
  return Math.sqrt(Math.pow(x2 - x1, 2) + Math.pow(y2 - y1, 2));
}

// Tìm đường đi ngắn nhất giữa hai beacon
function findShortestPath(startBeacon, endBeacon, position) {
  if (!(startBeacon in position) || !(endBeacon in position)) {
    throw new Error("One or both of the beacons do not exist.");
  }

  const distances = {};
  const previousNodes = {};
  const unvisited = new Set(Object.keys(position));

  // Khởi tạo khoảng cách ban đầu
  Object.keys(position).forEach((key) => {
    distances[key] = Infinity;
    previousNodes[key] = null;
  });
  distances[startBeacon] = 0;

  // Tìm đường đi ngắn nhất
  while (unvisited.size > 0) {
    const currentNode = Array.from(unvisited).reduce((minNode, node) =>
      distances[node] < distances[minNode] ? node : minNode
    );

    if (distances[currentNode] === Infinity) break;

    unvisited.delete(currentNode);

    Object.keys(position).forEach((neighbor) => {
      if (unvisited.has(neighbor)) {
        const alt =
          distances[currentNode] +
          calculateDistance(
            position[currentNode].Px,
            position[currentNode].Py,
            position[neighbor].Px,
            position[neighbor].Py
          );

        if (alt < distances[neighbor]) {
          distances[neighbor] = alt;
          previousNodes[neighbor] = currentNode;
        }
      }
    });
  }

  // Xây dựng đường đi từ endBeacon đến startBeacon
  const path = [];
  let currentNode = endBeacon;
  while (currentNode) {
    path.unshift(currentNode);
    currentNode = previousNodes[currentNode];
  }

  if (path[0] !== startBeacon) throw new Error("No path found.");
  return path;
}

// Vẽ đường đi ngắn nhất
function drawShortestPath(path, position) {
  const svg = document.getElementById("shortest-path");
  svg.innerHTML = ""; // Xóa các đường hiện có

  const imgElement = document.querySelector(".bg");
  const imgWidth = imgElement.naturalWidth;
  const imgHeight = imgElement.naturalHeight;

  const beaconPromises = path.map((beaconTagName) => {
    const data = position[beaconTagName];
    return {
      x: ((data.Px + 0.3) / imgWidth) * 100 * 10,
      y: ((data.Py + 0.2) / imgHeight) * 100 * 10,
    };
  });

  const beaconsCoordinates = Promise.resolve(beaconPromises);

  beaconsCoordinates.then((position) => {
    for (let i = 0; i < position.length - 1; i++) {
      const line = document.createElementNS(
        "http://www.w3.org/2000/svg",
        "line"
      );
      line.setAttribute("x1", `${position[i].x}%`);
      line.setAttribute("y1", `${position[i].y}%`);
      line.setAttribute("x2", `${position[i + 1].x}%`);
      line.setAttribute("y2", `${position[i + 1].y}%`);
      line.setAttribute("stroke", "#800000");
      line.setAttribute("stroke-width", "2");
      svg.appendChild(line);
    }
  });
}

// Lắng nghe sự kiện thay đổi từ Firebase
function listenForBeaconChanges() {
  let position = {};

  beacontagRef.on("child_added", (snapshot) => {
    position[snapshot.key] = snapshot.val();
    updatePath();
  });

  beacontagRef.on("child_changed", (snapshot) => {
    position[snapshot.key] = snapshot.val();
    updatePath();
  });

  beacontagRef.on("child_removed", (snapshot) => {
    delete position[snapshot.key];
    updatePath();
  });

  function updatePath() {
    const startBeacon = document.getElementById("start-beacon").value;
    const endBeacon = document.getElementById("end-beacon").value;

    if (startBeacon && endBeacon) {
      if (!(startBeacon in position) || !(endBeacon in position)) {
        alert("One or both of the selected beacons have been deleted.");
        const svg = document.getElementById("shortest-path");
        svg.innerHTML = ""; // Xóa các đường hiện có
        return;
      }

      try {
        const path = findShortestPath(startBeacon, endBeacon, position);
        drawShortestPath(path, position);
      } catch (error) {
        alert(error.message);
        const svg = document.getElementById("shortest-path");
        svg.innerHTML = ""; // Xóa các đường hiện có
      }
    }
  }

  // Xử lý sự kiện submit của form tìm đường
  document
    .getElementById("path-form")
    .addEventListener("submit", function (event) {
      event.preventDefault();
      updatePath();
    });
}

// Bắt đầu lắng nghe sự kiện thay đổi
listenForBeaconChanges();
