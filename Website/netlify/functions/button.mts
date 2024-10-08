import type { Context } from "@netlify/functions";
import { getData, updateData } from "../../data";

export default async (req: Request, context: Context) => {
  // get id query param from the request url using UrlSearchParams
  const url = new URL(req.url);
  const id = url.searchParams.get("id");

  const data = await getData();
  data[id].lastPressed = Date.now();
  await updateData(data);

  return new Response(data[id].lastPressed);
};
