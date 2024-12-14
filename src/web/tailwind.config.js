/** @type {import('tailwindcss').Config} */
export default {
    content: [
        "./src/**/*.vue",
        "./src/**/*.css"
    ],
    theme: {
        extend: {
            colors: {
                "c-light": "#ababab",
                "c-grey": "#545454",
                "c-dark": "#232323",
                "primary": "#251ee7",
                "secondary": "#32ade3"
            }
        },
    },
    plugins: [],
}

