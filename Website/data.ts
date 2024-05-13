import axios from "axios";

const JSONBIN_KEY = process.env.JSONBIN_KEY;
const JSONBIN_ID = "5eae3b8a47a2266b1471cb78";

interface PanelData {
  lastPressed: number;
  history: number[];
}

interface Data {
  water: PanelData;
  food: PanelData;
  poop: PanelData;
}

export const ID_TO_KEY = {
  0: "food",
  1: "poop",
  2: "water",
};

export async function getData(): Promise<Data> {
  const res = await axios({
    method: "GET",
    url: `https://api.jsonbin.io/v3/b/${JSONBIN_ID}`,
    headers: {
      "X-Access-Key": JSONBIN_KEY,
    },
  });
  return res.data.record as Data;
}

export async function updateData(data: Data): Promise<void> {
  await axios({
    method: "PUT",
    url: `https://api.jsonbin.io/v3/b/${JSONBIN_ID}`,
    headers: {
      "X-Access-Key": JSONBIN_KEY,
      'Content-Type': 'application/json'
    },
    data,
  });
}