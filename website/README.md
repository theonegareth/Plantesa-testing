![Screenshot of the website](assets/Screenshot%202025-06-03%20213417.png)

![Tree Icon](assets/tree-icon.png)
### Attribution

The tree icon used in this project is sourced from [Flaticon](https://www.flaticon.com/free-icon/tree-shape-of-straight-lines-like-a-computer-printed-circuit_40886?term=tree+circuit&page=1&position=3&origin=search&related_id=40886).

# Hosting with Firebase

This project uses [Firebase Hosting](https://firebase.google.com/products/hosting) to deploy and serve the website.

## Prerequisites

- [Node.js](https://nodejs.org/) installed
- [Firebase CLI](https://firebase.google.com/docs/cli) installed (`npm install -g firebase-tools`)
- A Firebase project (create one at [Firebase Console](https://console.firebase.google.com/))

## Setup

1. **Login to Firebase:**
    ```sh
    firebase login
    ```

2. **Initialize Firebase in your project:**
    ```sh
    firebase init hosting
    ```
    - Select your Firebase project.
    - Set the public directory (e.g., `dist` or `build`).
    - Configure as a single-page app if needed.

3. **Build your project** (if applicable):
    ```sh
    npm run build
    ```

4. **Deploy to Firebase Hosting:**
    ```sh
    firebase deploy
    ```

## Useful Commands

- `firebase serve` — Preview your site locally.
- `firebase deploy` — Deploy your site to production.

## Resources

- [Firebase Hosting Docs](https://firebase.google.com/docs/hosting)
- [Firebase CLI Reference](https://firebase.google.com/docs/cli)