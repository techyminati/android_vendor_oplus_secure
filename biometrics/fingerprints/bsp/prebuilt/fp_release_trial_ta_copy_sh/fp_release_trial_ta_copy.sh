#!/system/bin/sh

cfgfile=${PRJ_SOURCE_TREE}/vendor/oplus/secure/biometrics/fingerprints/bsp/prebuilt/fp_release_trial_ta_copy_sh/fp_config.ini
FP_FACE_RLS_TRIAL_DIR=""
FP_FACE_RLS_TRIAL_DIR1=""

for file in `find ${PRJ_SOURCE_TREE}/vendor/oplus/odm/${COMPILE_PLATFORM}/config/ -name "*_odm_list.conf"`; do
    cat $file | while read line || [[ -n ${line} ]]

    do
        line=${line//[$'\t\r\n']}

        cat $cfgfile | while read project path
        do
            if [ "$line"x = "$project"x ]; then
                echo "fp matched project: "$project

                path=${path//[$'\t\r\n']}
                newPath=$PRJ_SOURCE_TREE/vendor/oplus/secure/biometrics/fingerprints/bsp/prebuilt/project/$path

                if [ -d "$newPath" ];then
                    echo "fingerprint TA path: " $newPath

                    if [ "$1" = "mtk" ];then
                        #copy fingerprint ta bin file to euclid_images/odm/xxx_debug/vendor/app/mcRegistry directory
                        FP_FACE_RLS_TRIAL_DIR=${EUCLID_BIN_DIR}/odm/${project}_debug/vendor/app/mcRegistry
                        FP_FACE_RLS_TRIAL_DIR1=${EUCLID_BIN_DIR}/odm/${project}_debug/app/mcRegistry
                    else
                        #copy fingerprint ta bin file to euclid_images/odm/xxx_debug/vendor/firmware/ directory
                        FP_FACE_RLS_TRIAL_DIR=${EUCLID_BIN_DIR}/odm/${project}_debug/vendor/firmware
                        FP_FACE_RLS_TRIAL_DIR1=${EUCLID_BIN_DIR}/odm/${project}_debug/firmware
                    fi

                    if [ ! -d "$FP_FACE_RLS_TRIAL_DIR" ]; then
                        mkdir -p ${FP_FACE_RLS_TRIAL_DIR}
                    fi

                    #for /odm/xxx
                    echo "euclid_dst:${FP_FACE_RLS_TRIAL_DIR}"
                    cp $newPath/* ${FP_FACE_RLS_TRIAL_DIR}

                    if [ ! -d "$FP_FACE_RLS_TRIAL_DIR1" ]; then
                        mkdir -p ${FP_FACE_RLS_TRIAL_DIR1}
                    fi

                    #for /odm/vendor/xxx
                    echo "euclid_dst1:${FP_FACE_RLS_TRIAL_DIR1}"
                    cp $newPath/* ${FP_FACE_RLS_TRIAL_DIR1}
                fi
            fi
        done
    done
done
