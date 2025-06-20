firebase.initializeApp(firebaseConfig);

document.addEventListener('DOMContentLoaded', () => {
    const resultDiv = document.getElementById('latestResult');
    const imgContainer = document.getElementById('latestImage');
    const uploadTimeSpan = document.getElementById('uploadTime');
    const conditionDiv = document.getElementById('condition');
    const brownPercentageDiv = document.getElementById('brownPercentage');

    resultDiv.textContent = 'Firebase initialized successfully!';
    const storage = firebase.storage();
    const database = firebase.database();

    // Listen for latest image URL in the database
    const latestImageRef = database.ref('latest_image');
    latestImageRef.on('value', (snapshot) => {
        const url = snapshot.val();
        if (url) {
            imgContainer.innerHTML = `
                <div class="image-url">${url}</div>
                <img src="${url}" alt="Latest image" class="plant-image" />
            `;
        } else {
            imgContainer.textContent = 'No image available.';
        }
    });

    // Fetch condition and brown percentage from Realtime Database
    const brownPercentageRef = database.ref('brown_spots_percentage');
    const conditionRef = database.ref('leaf_conditions');
    const labeledPhotosRef = database.ref('labeled_photos');
    const imageUrlDiv = document.getElementById('imageUrl');

    labeledPhotosRef.on('value', (snapshot) => {
        const photos = snapshot.val();
        if (photos) {
            const keys = Object.keys(photos);
            keys.sort((a, b) => photos[b].timestamp - photos[a].timestamp);
            const latest = photos[keys[0]];
            if (latest.labeled_photo_url) {
                imgContainer.innerHTML = `
                <img src="${latest.labeled_photo_url}" alt="Latest labeled image" class="plant-image" />`;
                imageUrlDiv.textContent = latest.labeled_photo_url;
                uploadTimeSpan.textContent = latest.timestamp || 'Unknown';
            } else {
                imgContainer.textContent = 'No labeled image available.';
                imageUrlDiv.textContent = '';
                uploadTimeSpan.textContent = 'Unknown';
            }
        } else {
            imgContainer.textContent = 'No labeled image available.';
            imageUrlDiv.textContent = '';
            uploadTimeSpan.textContent = 'Unknown';
        }
    });

    conditionRef.on('value', (snapshot) => {
        const conditions = snapshot.val();
        if (conditions) {
            const keys = Object.keys(conditions);
            keys.sort((a, b) => conditions[b].timestamp - conditions[a].timestamp);
            const latest = conditions[keys[0]];
            conditionDiv.textContent = `Condition: ${latest.predicted_condition} (${latest.timestamp})`;
        } else {
            conditionDiv.textContent = 'Condition: --';
        }
    });

    brownPercentageRef.on('value', (snapshot) => {
        const brownPercentages = snapshot.val();
        if (brownPercentages) {
            const keys = Object.keys(brownPercentages);
            keys.sort((a, b) => brownPercentages[b].timestamp - brownPercentages[a].timestamp);
            const latest = brownPercentages[keys[0]];
            brownPercentageDiv.textContent = `Brown Percentage: ${latest.brown_percentage.toFixed(2)}% (${latest.timestamp})`;
        } else {
            brownPercentageDiv.textContent = 'Brown Percentage: --';
        }
    });

    const nextEstimateSpan = document.getElementById('nextEstimate');
    const nextPhotoEstimateRef = database.ref('next_photo_estimate');
    nextPhotoEstimateRef.on('value', (snapshot) => {
        const estimates = snapshot.val();
        if (estimates) {
            const keys = Object.keys(estimates);
            keys.sort((a, b) => estimates[b].timestamp - estimates[a].timestamp);
            const latest = estimates[keys[0]];
            nextEstimateSpan.textContent = `Next Estimated Picture: ${latest.next_estimated_time || '--'}`;
        } else {
            nextEstimateSpan.textContent = 'Next Estimated Picture: --';
        }
    });

    const usersDataRef = database.ref('SensorsData');

    // Listen for temperature, humidity, and soil moisture sensors
    const humidityDiv = document.getElementById('humidity');
    const temperatureDiv = document.getElementById('temperature');
    const moistureDiv = document.getElementById('moisture');
    const natriumDiv = document.getElementById('natrium');
    const phosporusDiv = document.getElementById('phosporus');
    const kaliumDiv = document.getElementById('kalium');
    const waterDiv = document.getElementById('waterlevel');
    const wateranalogDiv = document.getElementById('wateranalog');

    usersDataRef.on('value', (snapshot) => {
        const users = snapshot.val();
        if (users) {
            const keys = Object.keys(users);
            const latest = users[keys[keys.length - 1]];

            humidityDiv.textContent = `Humidity: ${latest.humidity ?? '--'}%`;
            temperatureDiv.textContent = `Temperature: ${latest.temperature ?? '--'}°C`;
            moistureDiv.textContent = `Soil Moisture: ${latest.moisture ?? '--'}%`;

            natriumDiv.textContent = `Natrium: ${latest.natrium ?? '--'}`;
            phosporusDiv.textContent = `Phosphorus: ${latest.phosphorus ?? '--'}`;
            kaliumDiv.textContent = `Kalium: ${latest.kalium ?? '--'}`;

            waterDiv.textContent = `Water Level: ${latest.waterlevel ?? '--'}`;

            wateranalogDiv.textContent = `Water Analog: ${latest.wateranalog ?? '--'}`;
        } else {
            humidityDiv.textContent = 'Humidity: --';
            temperatureDiv.textContent = 'Temperature: --';
            moistureDiv.textContent = 'Soil Moisture: --';

            natriumDiv.textContent = 'Natrium: --';
            phosporusDiv.textContent = 'Phosphorus: --';
            kaliumDiv.textContent = 'Kalium: --';

            waterDiv.textContent = 'Water Level: --';

            wateranalogDiv.textContent = 'Water Analog: --';
        }
    });

    const waterLevelPercentageDiv = document.getElementById('waterLevelPercentage');
    const estimatedRefillDiv = document.getElementById('estimatedRefill');

    // Listen for water level percentage
    const waterLevelPercentageRef = database.ref('waterlevel_percentage');
    waterLevelPercentageRef.on('value', (snapshot) => {
        const data = snapshot.val();
        if (data) {
            const keys = Object.keys(data);
            keys.sort((a, b) => data[b].timestamp - data[a].timestamp);
            const latest = data[keys[0]];
            waterLevelPercentageDiv.textContent = `Water Level Percentage: ${latest.waterlevel_percentage ?? '--'}% (${latest.timestamp || '--'})`;
        } else {
            waterLevelPercentageDiv.textContent = 'Water Level Percentage: --';
        }
    });

    // Listen for estimated refill time
    const estimatedRefillRef = database.ref('estimated_refill_time');
    estimatedRefillRef.on('value', (snapshot) => {
        const data = snapshot.val();
        if (data) {
            const keys = Object.keys(data);
            keys.sort((a, b) => data[b].timestamp - data[a].timestamp);
            const latest = data[keys[0]];
            if (latest.estimated_refill_time !== undefined) {
                const totalMinutes = latest.estimated_refill_time;
                const days = Math.floor(totalMinutes / (24 * 60));
                const remainingMinutesAfterDays = totalMinutes % (24 * 60);
                const hours = Math.floor(remainingMinutesAfterDays / 60);
                const minutes = remainingMinutesAfterDays % 60;
                estimatedRefillDiv.textContent = `Estimated Refill Time: ${days}d ${hours}h ${minutes}m (${latest.timestamp || '--'})`;
            } else {
                estimatedRefillDiv.textContent = 'Estimated Refill Time: --';
            }
        } else {
            estimatedRefillDiv.textContent = 'Estimated Refill Time: --';
        }
    });

    const pump1Div = document.getElementById('pump1');
    const pump2Div = document.getElementById('pump2');
    const pump3Div = document.getElementById('pump3');
    const lightStatusDiv = document.getElementById('lightStatus');
    const fanStatusDiv = document.getElementById('fanStatus');

    usersDataRef.on('value', (snapshot) => {
        const users = snapshot.val();
        if (users) {
            const keys = Object.keys(users);
            const latest = users[keys[keys.length - 1]];

            if (latest.relays) {
                pump1Div.textContent = latest.relays.pump1 === "ON" ? "Pump 1: Active" : "Pump 1: Inactive";
                pump2Div.textContent = latest.relays.pump2 === "ON" ? "Pump 2: Active" : "Pump 2: Inactive";
                pump3Div.textContent = latest.relays.pump3 === "ON" ? "Pump 3: Active" : "Pump 3: Inactive";
                lightStatusDiv.textContent = latest.relays.light === "ON" ? "Light: On" : "Light: Off";
                fanStatusDiv.textContent = latest.relays.fan === "ON" ? "Fan: On" : "Fan: Off";
            } else {
                pump1Div.textContent = 'Pump 1: --';
                pump2Div.textContent = 'Pump 2: --';
                pump3Div.textContent = 'Pump 3: --';
                lightStatusDiv.textContent = 'Light: --';
                fanStatusDiv.textContent = 'Fan: --';
            }
        } else {
            pump1Div.textContent = 'Pump 1: --';
            pump2Div.textContent = 'Pump 2: --';
            pump3Div.textContent = 'Pump 3: --';
            lightStatusDiv.textContent = 'Light: --';
            fanStatusDiv.textContent = 'Fan: --';
        }
    });
});
