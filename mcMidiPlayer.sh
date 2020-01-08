#! /bin/sh

dir="${HOME}/.mcMidiPlayer"
if [ -d ${dir} ] ; then
    echo "Le dossier paramètres ${dir} existe"
else    
	mkdir ${dir}
	echo "Le dossier paramètres ${dir} est créé"
	cp  /usr/local/share/mcMidiPlayer/parametres.conf ${dir}	
fi
dir="${HOME}/mcMidiPlayer"
if [ -d ${dir} ] ; then
    echo "Le dossier de travail ${dir} existe"
else    
	mkdir ${dir}
	echo "Le dossier de travail ${dir} est créé"
fi
/usr/local/bin/mcMidiPlayer.bin ${HOME}