#!/bin/bash

# Script pour compiler OpenSSL pour toutes les architectures Android
# Basé sur la même logique que ndk_setup.sh pour SDL

set -e

# Configuration
OPENSSL_VERSION="3.6.0"
ANDROID_API="21"
ANDROID_NDK_ROOT="/usr/lib/android-ndk"
OPENSSL_URL="https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz"

# Répertoires
WORK_DIR="/openssl"
INSTALL_DIR="/openssl-lib"

# Architectures à compiler (comme dans ndk_setup.sh)
declare -A ARCHS=(
    ["arm64-v8a"]="android-arm64:aarch64-linux-android${ANDROID_API}"
    ["armeabi-v7a"]="android-arm:armv7a-linux-androideabi${ANDROID_API}"
    ["x86"]="android-x86:i686-linux-android${ANDROID_API}"
    ["x86_64"]="android-x86_64:x86_64-linux-android${ANDROID_API}"
)

echo "=== Compilation OpenSSL ${OPENSSL_VERSION} pour Android ==="
echo "NDK: ${ANDROID_NDK_ROOT}"
echo "API Level: ${ANDROID_API}"
echo

# Fonction pour télécharger et extraire OpenSSL
setup_openssl() {
    echo "📦 Téléchargement et extraction d'OpenSSL..."
    mkdir -p ${WORK_DIR}
    cd ${WORK_DIR}
    
    if [ ! -f "openssl-${OPENSSL_VERSION}.tar.gz" ]; then
        wget ${OPENSSL_URL}
    fi
    
    if [ ! -d "openssl-${OPENSSL_VERSION}" ]; then
        tar xzf "openssl-${OPENSSL_VERSION}.tar.gz"
    fi
    
    echo "✅ OpenSSL extrait dans ${WORK_DIR}/openssl-${OPENSSL_VERSION}"
}

# Fonction pour compiler OpenSSL pour une architecture donnée
compile_arch() {
    local arch=$1
    local config_target=$2
    local compiler_prefix=$3
    
    echo
    echo "🔨 Compilation pour ${arch}..."
    echo "   Target: ${config_target}"
    echo "   Compiler: ${compiler_prefix}-clang"
    
    # Créer un répertoire séparé pour chaque architecture
    local build_dir="${WORK_DIR}/openssl-${arch}"
    rm -rf ${build_dir}
    mkdir -p ${build_dir}
    cp -r ${WORK_DIR}/openssl-${OPENSSL_VERSION}/* ${build_dir}/
    
    cd ${build_dir}
    
    # Configuration des variables d'environnement
    export PATH="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"
    export CC="${compiler_prefix}-clang"
    export CXX="${compiler_prefix}-clang++"
    export AR="llvm-ar"
    export RANLIB="llvm-ranlib"
    export STRIP="llvm-strip"
    
    # Installation prefix
    local install_prefix="${WORK_DIR}/install/${arch}"
    
    # Configuration OpenSSL
    ./Configure ${config_target} \
        -D__ANDROID_API__=${ANDROID_API} \
        --prefix=${install_prefix} \
        --openssldir=${install_prefix} \
        no-shared \
        no-tests
    
    # Compilation
    make clean 2>/dev/null || true
    make -j$(nproc)
    make install_sw
    
    echo "✅ ${arch} compilé avec succès dans ${install_prefix}"
}

# Fonction pour organiser les bibliothèques dans la structure finale
organize_libs() {
    echo
    echo "📁 Organisation des bibliothèques..."
    
    # Créer la structure de répertoires
    mkdir -p ${INSTALL_DIR}/{arm64-v8a,armeabi-v7a,x86,x86_64}
    mkdir -p ${INSTALL_DIR}/include
    
    # Copier les bibliothèques pour chaque architecture
    for arch in "${!ARCHS[@]}"; do
        local src_dir="${WORK_DIR}/install/${arch}"
        local dst_dir="${INSTALL_DIR}/${arch}"
        
        if [ -d "${src_dir}/lib" ]; then
            echo "   Copie ${arch}..."
            cp ${src_dir}/lib/*.a ${dst_dir}/ 2>/dev/null || true
            
            # Copier les headers une seule fois (ils sont identiques pour toutes les archs)
            if [ "${arch}" == "arm64-v8a" ] && [ -d "${src_dir}/include" ]; then
                cp -r ${src_dir}/include/* ${INSTALL_DIR}/include/
            fi
        else
            echo "   ⚠️  Attention: ${src_dir}/lib n'existe pas"
        fi
    done
    
    echo "✅ Organisation terminée dans ${INSTALL_DIR}"
}

# Fonction pour afficher un résumé
show_summary() {
    echo
    echo "📊 Résumé de la compilation:"
    echo "================================"
    
    for arch in "${!ARCHS[@]}"; do
        local dst_dir="${INSTALL_DIR}/${arch}"
        if [ -d "${dst_dir}" ]; then
            local lib_count=$(ls ${dst_dir}/*.a 2>/dev/null | wc -l)
            echo "   ${arch}: ${lib_count} bibliothèque(s)"
            ls ${dst_dir}/*.a 2>/dev/null | sed 's/.*\//     - /'
        fi
    done
    
    if [ -d "${INSTALL_DIR}/include" ]; then
        local header_count=$(find ${INSTALL_DIR}/include -name "*.h" | wc -l)
        echo "   Headers: ${header_count} fichier(s)"
    fi
}

# Fonction pour nettoyer
clean() {
    echo "🧹 Nettoyage des fichiers temporaires..."
    rm -rf ${WORK_DIR}/openssl-* 2>/dev/null || true
    echo "✅ Nettoyage terminé"
}

# Vérification des prérequis
check_requirements() {
    if [ ! -d "${ANDROID_NDK_ROOT}" ]; then
        echo "❌ Erreur: Android NDK non trouvé dans ${ANDROID_NDK_ROOT}"
        echo "   Installez-le avec: apt install google-android-ndk-r20-installer"
        exit 1
    fi
    
    if [ ! -d "${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin" ]; then
        echo "❌ Erreur: Toolchain LLVM non trouvée dans le NDK"
        exit 1
    fi
    
    echo "✅ Prérequis vérifiés"
}

# Fonction principale
main() {
    echo "Vérification des prérequis..."
    check_requirements
    
    # echo
    # read -p "Voulez-vous continuer avec la compilation ? (y/N) " -n 1 -r
    # echo
    # if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    #     echo "Compilation annulée."
    #     exit 0
    # fi
    
    # Téléchargement et extraction
    setup_openssl
    
    # Compilation pour chaque architecture
    for arch in "${!ARCHS[@]}"; do
        IFS=':' read -r config_target compiler_prefix <<< "${ARCHS[$arch]}"
        compile_arch "${arch}" "${config_target}" "${compiler_prefix}"
    done
    
    # Organisation des bibliothèques
    organize_libs
    
    # Résumé
    show_summary
    
    # Nettoyage optionnel
    # echo
    # read -p "Voulez-vous nettoyer les fichiers temporaires ? (y/N) " -n 1 -r
    # echo
    # if [[ $REPLY =~ ^[Yy]$ ]]; then
    #     clean
    # fi
    
    echo
    echo "🎉 Compilation OpenSSL terminée avec succès !"
    echo "   Bibliothèques disponibles dans: ${INSTALL_DIR}"
}

# Gestion des arguments
case "${1:-}" in
    "clean")
        clean
        exit 0
        ;;
    "check")
        check_requirements
        exit 0
        ;;
    "help"|"-h"|"--help")
        echo "Usage: $0 [clean|check|help]"
        echo "  clean : Nettoie les fichiers temporaires"
        echo "  check : Vérifie les prérequis"
        echo "  help  : Affiche cette aide"
        exit 0
        ;;
    "")
        main
        ;;
    *)
        echo "Option inconnue: $1"
        echo "Utilisez '$0 help' pour voir les options disponibles"
        exit 1
        ;;
esac