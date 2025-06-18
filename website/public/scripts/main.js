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
    const latestImageDiv = document.getElementById('latestImage');
    if (latestImageDiv && imageUrl) {
        latestImageDiv.innerHTML = `<img src="${imageUrl}" alt="Latest Plant Image" />`;
    }

    // Listen for latest image URL in the database
    const latestImageRef = database.ref('latest_image');
    latestImageRef.on('value', (snapshot) => {
        const url = snapshot.val();
        if (url) {
            console.log('latest_image URL from DB:', url); // Debug line
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
            keys.sort((a, b) => {
                const ta = photos[a].timestamp;
                const tb = photos[b].timestamp;
                return ta < tb ? 1 : -1;
            });
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
                // Sort by timestamp descending
                keys.sort((a, b) => {
                    const ta = conditions[a].timestamp;
                    const tb = conditions[b].timestamp;
                    return ta < tb ? 1 : -1;
                });
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
                keys.sort((a, b) => {
                    const ta = brownPercentages[a].timestamp;
                    const tb = brownPercentages[b].timestamp;
                    return ta < tb ? 1 : -1;
                });
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
            // Get the latest estimate by sorting keys by timestamp
            const keys = Object.keys(estimates);
            keys.sort((a, b) => {
                const ta = estimates[a].timestamp;
                const tb = estimates[b].timestamp;
                return ta < tb ? 1 : -1;
            });
            const latest = estimates[keys[0]];
            nextEstimateSpan.textContent = `Next Estimated Picture: ${latest.next_estimated_time || '--'}`;
        } else {
            nextEstimateSpan.textContent = 'Next Estimated Picture: --';
        }
    });

    // Listen for temperature, humidity, and soil moisture sensors
    const humidityDiv = document.getElementById('humidity');
    const temperatureDiv = document.getElementById('temperature');
    const moistureDiv = document.getElementById('moisture');
    const natriumDiv = document.getElementById('natrium');
    const phosporusDiv = document.getElementById('phosporus'); // update HTML id to 'phosphorus' if you want to match Arduino
    const kaliumDiv = document.getElementById('kalium');
    const waterDiv = document.getElementById('water'); // or 'waterLevel' if that's your HTML id

    const usersDataRef = database.ref('SensorsData');
    usersDataRef.on('value', (snapshot) => {
        const users = snapshot.val();
        if (users) {
            const keys = Object.keys(users);
            const latest = users[keys[keys.length - 1]];

            humidityDiv.textContent = `Humidity: ${latest.humidity !== undefined ? latest.humidity + '%' : '--'}`;
            temperatureDiv.textContent = `Temperature: ${latest.temperature !== undefined ? latest.temperature + '°C' : '--'}`;
            moistureDiv.textContent = `Soil Moisture: ${latest.moisture !== undefined ? latest.moisture + '%' : '--'}`;

            natriumDiv.textContent = `Natrium: ${latest.natrium !== undefined ? latest.natrium : '--'}`;
            phosporusDiv.textContent = `Phosphorus: ${latest.phosphorus !== undefined ? latest.phosphorus : '--'}`; // match spelling
            kaliumDiv.textContent = `Kalium: ${latest.kalium !== undefined ? latest.kalium : '--'}`;

            waterDiv.textContent = `Water Level: ${latest.waterlevel !== undefined ? latest.waterlevel : '--'}`;
        } else {
            humidityDiv.textContent = 'Humidity: --';
            temperatureDiv.textContent = 'Temperature: --';
            moistureDiv.textContent = 'Soil Moisture: --';

            natriumDiv.textContent = 'Natrium: --';
            phosporusDiv.textContent = 'Phosphorus: --';
            kaliumDiv.textContent = 'Kalium: --';

            waterDiv.textContent = 'Water Level: --';
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
            keys.sort((a, b) => {
                const ta = data[a].timestamp;
                const tb = data[b].timestamp;
                return ta < tb ? 1 : -1;
            });
            const latest = data[keys[0]];
            waterLevelPercentageDiv.textContent = `Water Level Percentage: ${latest.waterlevel_percentage !== undefined ? latest.waterlevel_percentage + '%' : '--'} (${latest.timestamp || '--'})`;
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
            keys.sort((a, b) => {
                const ta = data[a].timestamp;
                const tb = data[b].timestamp;
                return ta < tb ? 1 : -1;
            });
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

    // Listen for pump status
    const pumpStatusRef = database.ref('pump_status');
    pumpStatusRef.on('value', (snapshot) => {
        const data = snapshot.val();
        if (data) {
            pump1Div.textContent = `Pump 1 Status: ${data.pump1 !== undefined ? data.pump1 : '--'}`;
            pump2Div.textContent = `Pump 2 Status: ${data.pump2 !== undefined ? data.pump2 : '--'}`;
            pump3Div.textContent = `Pump 3 Status: ${data.pump3 !== undefined ? data.pump3 : '--'}`;
        } else {
            pump1Div.textContent = 'Pump 1 Status: --';
            pump2Div.textContent = 'Pump 2 Status: --';
            pump3Div.textContent = 'Pump 3 Status: --';
        }
    });

    // Listen for light status
    const lightStatusRef = database.ref('light_status');
    lightStatusRef.on('value', (snapshot) => {
        const status = snapshot.val();
        lightStatusDiv.textContent = `Light Status: ${status !== undefined ? status : '--'}`;
    });

    // Listen for fan status
    const fanStatusRef = database.ref('fan_status');
    fanStatusRef.on('value', (snapshot) => {
        const status = snapshot.val();
        fanStatusDiv.textContent = `Fan Status: ${status !== undefined ? status : '--'}`;
    });
});