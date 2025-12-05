package main

import (
    "fmt"
    "net/http"
	"os"
	"os/exec"
	"io"
	"log"
	"regexp"
	"archive/zip"
)

func getEmbedding(w http.ResponseWriter, r *http.Request){
		// Read the file sent over the http request
		file, header, err := r.FormFile("file")
		if err != nil {
			http.Error(w, "missing file", http.StatusBadRequest)
			return
		}
		defer file.Close()

		fmt.Fprintf(w, "file name is %s\n", header.Filename)

		// Copy the file to input.gml
		dst, err1 := os.Create("input.gml")
		if err1 != nil {
			http.Error(w, "Can't create a new file", http.StatusBadRequest)
 			log.Fatal(err1)
		}

		defer dst.Close()

		_, err = io.Copy(dst, file)

		// Execute the algorithm
		out, err2 := exec.Command("/bin/sh", "script.sh", "input.gml").Output()
		if err2 != nil {
			log.Fatal(err2)
		}
		fmt.Fprintf(w, "%s", out)

		// Create a result file
		result, errCreate := os.Create("result.svg")
		fmt.Fprintf(w, "%s", out)
		if errCreate != nil {
			http.Error(w, "Can't create a new file", http.StatusBadRequest)
 			log.Fatal(errCreate)
		}

		defer result.Close()

		// Read the directory where there is the result
		files, err3 := os.ReadDir(".")
		if err3 != nil {
			log.Fatal(err) 
		}

		// Create a zip package for the results 

		zw := zip.NewWriter(w)
		defer zw.Close()

		for _, dirEntry := range files {
			match, _ := regexp.MatchString("input[^.]*.svg", dirEntry.Name())
			var name string
			if(match){
				f, err := os.Open(dirEntry.Name())
				if err != nil {
					http.Error(w, "open error", http.StatusInternalServerError)
					return
				}
				name = "input.svg"
				if(dirEntry.Name() != "input.svg"){
					name = "result.svg"
				}
				fw, err := zw.Create(name)
				if err != nil {
					f.Close()
					http.Error(w, "zip error", http.StatusInternalServerError)
					return
				}
				if _, err := io.Copy(fw, f); err != nil {
					f.Close()
					http.Error(w, "copy error", http.StatusInternalServerError)
					return
				}
				fmt.Fprintf(w, "file: %s \n", f.Name())
				f.Close()
			} 
		}
		

		w.Header().Set("Content-Type", "application/zip")
		w.Header().Set("Content-Disposition", `attachment; filename="files.zip"`)

}
func main() {

	http.HandleFunc("/embedding", getEmbedding)
	http.ListenAndServe(":8087", nil)
}
