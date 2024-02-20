function getPrefix() {
    const url = new URL(window.location.href);
    return url.href + "assets/"
}