import React from "react";
import ReactDOM from "react-dom/client";
import App from "./app.jsx";
import registerServiceWorker from "./register-service-worker.js";

import "./index.css";

ReactDOM.createRoot(document.getElementById("root")).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);

registerServiceWorker();
