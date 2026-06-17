{
  description = "A C++ library for glob pattern matching on strings and filesystems";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages = {
          default = self.packages.${system}.glob-cpp;
          glob-cpp = pkgs.stdenv.mkDerivation {
            pname = "glob-cpp";
            version = "1.1.1";
            src = self;
            nativeBuildInputs = [ pkgs.cmake ];
            buildInputs = [ pkgs.boost ];
            cmakeFlags = [
              "-DBUILD_UNIT_TESTS=OFF"
              "-DBUILD_EXAMPLES=OFF"
            ];
            # Header-only library: install the include tree and the CMake
            # config so downstream consumers can find_package(glob-cpp).
            installPhase = ''
              runHook preInstall
              mkdir -p "$out/include"
              cp -r "$src/include/glob-cpp" "$out/include/"
              mkdir -p "$out/lib/cmake/glob-cpp"
              cat > "$out/lib/cmake/glob-cpp/glob-cpp-config.cmake" <<'CMAKE'
              add_library(glob-cpp INTERFACE IMPORTED)
              set_target_properties(glob-cpp PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "''${CMAKE_CURRENT_LIST_DIR}/../../../include"
              )
              find_package(Boost REQUIRED COMPONENTS filesystem)
              target_include_directories(glob-cpp INTERFACE ''${Boost_INCLUDE_DIRS})
              target_link_libraries(glob-cpp INTERFACE ''${Boost_FILESYSTEM_LIBRARY} ''${Boost_SYSTEM_LIBRARY})
              CMAKE
              runHook postInstall
            '';
          };
        };

        devShells = {
          default = self.devShells.${system}.glob-cpp;
          glob-cpp = pkgs.mkShell {
            inputsFrom = [ self.packages.${system}.glob-cpp ];
            packages = with pkgs; [
              cmake
              boost
              gtest
            ];
            shellHook = ''
              echo "glob-cpp dev shell"
              echo "  mkdir build && cd build && cmake .. -DBUILD_UNIT_TESTS=ON -DBUILD_EXAMPLES=ON && make && make test"
            '';
          };
        };
      }
    );
}
