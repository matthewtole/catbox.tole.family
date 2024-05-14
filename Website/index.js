const { DateTime } = luxon;

const refresh = document.getElementById("refresh");

const panels = {
  poop: {
    element: document.querySelector(".panel[data-id='poop']"),
    delay: 20000,
  },
  water: {
    element: document.querySelector(".panel[data-id='water']"),
    delay: 20000,
  },
  food: {
    element: document.querySelector(".panel[data-id='food']"),
    delay: 20000,
  },
};

function updatePanel(id, data) {
  const element = panels[id].element;
  element.querySelector(".time").innerText = DateTime.fromMillis(
    data.lastPressed
  ).toRelative();
  if (Date.now() - data.lastPressed >= panels[id].delay) {
    element.setAttribute("data-mode", "on");
  } else {
    element.setAttribute("data-mode", "off");
  }
}

async function fetchData() {
  if (refresh.classList.contains("active")) {
    return;
  }
  refresh.classList.add("active");
  const res = await axios.get("/.netlify/functions/data");
  updatePanel("poop", res.data.poop);
  updatePanel("water", res.data.water);
  updatePanel("food", res.data.food);
  refresh.classList.remove("active");
}

function boot() {
  setInterval(fetchData, 5000);
  fetchData().catch((err) => console.error(err));
  refresh.addEventListener("click", fetchData);
}

boot();
