export default async (req, context) => {
  // get id query param from the request url using UrlSearchParams
  const url = new URL(req.url);
  const id = url.searchParams.get("id");
  return new Response("Hello from button.mjs! id: " + id);
};
