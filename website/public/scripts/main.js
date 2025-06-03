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
            console.log('latest_image URL from DB:', url); // Debug line
            imgContainer.innerHTML = `<div style="word-break:break-all;font-size:0.9em;color:#888;">${url}</div>
                <img src="${url}" alt="Latest image" style="max-width:100%; height:auto;">`;
        } else {
            imgContainer.textContent = 'No image available.';
        }
    });

    // Fetch condition and brown percentage from Realtime Database
    const brownPercentageRef = database.ref('brown_spots_percentage');
    const conditionRef = database.ref('leaf_conditions');
    const labeledPhotosRef = database.ref('labeled_photos');

        labeledPhotosRef.on('value', (snapshot) => {
            const photos = snapshot.val();
            if (photos) {
                const keys = Object.keys(photos);
                // Sort by timestamp descending
                keys.sort((a, b) => {
                    const ta = photos[a].timestamp;
                    const tb = photos[b].timestamp;
                    return ta < tb ? 1 : -1;
                });
                const latest = photos[keys[0]];
                if (latest.labeled_photo_url) {
                    console.log('Using labeled_photo_url from labeled_photos:', latest.labeled_photo_url);
                    imgContainer.innerHTML = `<div style="word-break:break-all;font-size:0.9em;color:#888;">${latest.labeled_photo_url}</div>
                        <img src="${latest.labeled_photo_url}" alt="Latest labeled image" style="max-width:100%; height:auto;">`;
                    uploadTimeSpan.textContent = latest.timestamp || 'Unknown';
                } else {
                    imgContainer.textContent = 'No labeled image available.';
                    uploadTimeSpan.textContent = 'Unknown';
                }
            } else {
                imgContainer.textContent = 'No labeled image available.';
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
});
