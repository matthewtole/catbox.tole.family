const { default: axios } = require("axios");

document.addEventListener("load", () => {
  axios
    .get("/netlify/functions/data")
    .then((res) => console.log(res.data.record));
});
