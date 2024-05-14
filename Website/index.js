const panelPoop = document.querySelector(".panel[data-id='poop']");
const panelWater = document.querySelector(".panel[data-id='water']");
const panelFood = document.querySelector(".panel[data-id='food']");

async function fetchData() {
  const res = await axios.get("/.netlify/functions/data");
  panelPoop.querySelector(".time").innerText = luxon.DateTime.fromMillis(
    res.data.poop.lastPressed
  ).toLocaleString(DateTime.DATETIME_MED);
  panelWater.querySelector(".time").innerText = luxon.DateTime.fromMillis(
    res.data.water.lastPressed
  ).toLocaleString(DateTime.DATETIME_MED);
  panelFood.querySelector(".time").innerText = luxon.DateTime.fromMillis(
    res.data.food.lastPressed
  ).toLocaleString(DateTime.DATETIME_MED);
}

fetchData().catch((err) => console.error(err));
