import { createTheme } from "@mui/material/styles";

const theme = createTheme({

    palette:{

        mode:"dark",

        primary:{
            main:"#1976d2"
        },

        success:{
            main:"#2e7d32"
        },

        error:{
            main:"#d32f2f"
        },

        warning:{
            main:"#ed6c02"
        },

        background:{

            default:"#0f172a",

            paper:"#1e293b"

        }

    },

    shape:{

        borderRadius:18

    },

    typography:{

        fontFamily:"Roboto",

        h3:{
            fontWeight:70
        },

        h5:{
            fontWeight:60
        }

    }

});

export default theme;