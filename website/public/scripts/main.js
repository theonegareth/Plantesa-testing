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
            moistureDiv.textContent = `Soil Moisture: ${latest.soil_moisture !== undefined ? latest.soil_moisture + '%' : '--'}`;
            
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
});