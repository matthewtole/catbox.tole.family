const panelPoop = document.querySelector(".panel[data-id='poop']");
const panelWater = document.querySelector(".panel[data-id='water']");
const panelFood = document.querySelector(".panel[data-id='food']");

async function fetchData() {
  const res = await axios.get("/.netlify/functions/data");
  panelPoop.querySelector("time").innerText = res.data.poop.lastPressed;
  panelWater.querySelector("time").innerText = res.data.water.lastPressed;
  panelFood.querySelector("time").innerText = res.data.food.lastPressed;
}

fetchData().catch((err) => console.error(err));
