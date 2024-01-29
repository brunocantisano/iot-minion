function hasWhiteSpace(s) {
    return s.indexOf(' ') >= 0;
}

function Validate(oForm, validFileExtensions) {
    var arrInputs = oForm.getElementsByTagName("input");
    for (var i = 0; i < arrInputs.length; i++) {
        var oInput = arrInputs[i];
        if (oInput.type == "file") {
            var sFileName = oInput.value;
            if (sFileName.length > 0) {
                var blnValid = false;
                for (var j = 0; j < validFileExtensions.length; j++) {
                    var sCurExtension = validFileExtensions[j];
                    if (sFileName.substr(sFileName.length - sCurExtension.length, sCurExtension.length).toLowerCase() == sCurExtension.toLowerCase()) {
                        blnValid = true;
                        break;
                    }
                }
                if (hasWhiteSpace(sFileName)) {
                    $('#message').html("Desculpe, o arquivo: <b>" + sFileName + "</b> é inválido, o arquivo não pode conter espaços.");
                    $("#myToast").toast("show");
                    return false;
                }
                if (!blnValid) {
                    $('#message').html("Desculpe, o arquivo: <b>" + sFileName + "</b> é inválido, o arquivo deve ter a(s) extensão(ões): <b>" + validFileExtensions.join(", ") + "</b>.");
                    $("#myToast").toast("show");
                    return false;
                }
            }
        }
    }
    return true;
}

function deleteFile(tipo, filename, token) {
    const url = '/deleteFile?type=' + tipo;
    var promiseResponse = fetch(url, {
        method: "DELETE",
        headers: {
            "Content-Type": "application/json",
            "Accept": "text/plain",
            "Authorization": "Basic "+token
        },
        body: JSON.stringify({
            midia: filename
        }),
    });
    promiseResponse.then((data) => {
        //console.log(data);
    }).catch((err) => {
        console.log(err);
    }).finally(() => {
        location.href = location.href;
    })
}

function uploadFile(tipo, token) {
    let validExtensions = [];
    let url = '';
    if(tipo == "sdcard") {
        validExtensions = [".png", ".jpg", ".bmp", ".gif", ".wav", ".mp3"];
        url = '/uploadSdcard';
    } else {
        validExtensions = [".crt"];
        url = '/uploadStorage';
    }
    var inpFile = document.getElementById('inputFile');
    if (inpFile.value == '') {
        alert('Você precisa escolher um arquivo antes de clicar em upload!');
        return false;
    } else if (Validate(document.getElementById('myForm'), validExtensions)) {
        var file = inpFile.files[0];
        const formData = new FormData();
        formData.append("file", file, file.name);
        var promiseResponse = fetch(url, {
            method: "POST",
            headers: {
                "Authorization": "Basic "+token
            },
            body: formData,
        });
        promiseResponse.then((data) => {
            //console.log(data);
        }).catch((err) => {
            console.log(err);
        }).finally(() => {
            location.href = location.href;
        })
    }
}
