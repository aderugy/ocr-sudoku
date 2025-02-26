import './App.css';
import Cube from "./components/Cube";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faGithub} from "@fortawesome/free-brands-svg-icons";

function App() {
    const cube_size = 500;

    return (
        <div className="App">
            <main className="main">
                <div className={"spinning-cube"}>
                    <div style={{ width: cube_size, height: cube_size }}>
                        <Cube />
                    </div>
                </div>
                <div className={"main-content"} >
                    <div className={"project-name"}>
                        <h2 className={"title-sudo"}>SUDO</h2>
                        <h2 className={"title-cul"}>- CUL</h2>
                        <h4 className={"subtitle"}>An AI powered sudoku solver.</h4>
                    </div>
                    <nav className={"nav"}>
                        <a href={"https://github.com/Michael-Rousseau/OCR_Sudocul"} className="github-button nav-button" target="_blank" rel="noopener noreferrer">
                            <FontAwesomeIcon size={'2xl'} icon={faGithub} /><div>View on GitHub</div>
                        </a>
                    </nav>
                </div>
            </main>

        </div>
    );
}

export default App;
