import './Cube.css'; // Make sure to create a corresponding CSS file
import CubeFace from "./CubeFace";

const Cube = () => {
    return <div className="cube">
        <CubeFace className="face front"></CubeFace>
        <CubeFace className="face back"></CubeFace>
        <CubeFace className="face left"></CubeFace>
        <CubeFace className="face right"></CubeFace>
        <CubeFace className="face top"></CubeFace>
        <CubeFace className="face bottom"></CubeFace>
    </div>

};

export default Cube;