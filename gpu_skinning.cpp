#include "framework.h"

using namespace prototyper;

int main( int argc, char** argv )
{
  map<string, string> args;

  for( int c = 1; c < argc; ++c )
  {
    args[argv[c]] = c + 1 < argc ? argv[c + 1] : "";
    ++c;
  }

  cout << "Arguments: " << endl;
  for_each( args.begin(), args.end(), []( pair<string, string> p )
  {
    cout << p.first << " " << p.second << endl;
  } );

  uvec2 screen( 0 );
  bool fullscreen = false;
  bool silent = false;
  string title = "Basic CPP to get started";

  /*
   * Process program arguments
   */

  stringstream ss;
  ss.str( args["--screenx"] );
  ss >> screen.x;
  ss.clear();
  ss.str( args["--screeny"] );
  ss >> screen.y;
  ss.clear();

  if( screen.x == 0 )
  {
    screen.x = 1280;
  }

  if( screen.y == 0 )
  {
    screen.y = 720;
  }

  try
  {
    args.at( "--fullscreen" );
    fullscreen = true;
  }
  catch( ... ) {}

  try
  {
    args.at( "--help" );
    cout << title << ", written by Marton Tamas." << endl <<
         "Usage: --silent      //don't display FPS info in the terminal" << endl <<
         "       --screenx num //set screen width (default:1280)" << endl <<
         "       --screeny num //set screen height (default:720)" << endl <<
         "       --fullscreen  //set fullscreen, windowed by default" << endl <<
         "       --help        //display this information" << endl;
    return 0;
  }
  catch( ... ) {}

  try
  {
    args.at( "--silent" );
    silent = true;
  }
  catch( ... ) {}

  /*
   * Initialize the OpenGL context
   */

  framework frm;
  frm.init( screen, title, fullscreen );
  frm.set_vsync( true );

  //set opengl settings
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glFrontFace( GL_CCW );
  glEnable( GL_CULL_FACE );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClearDepth( 1.0f );

  frm.get_opengl_error();

  /*
   * Set up mymath
   */

  camera<float> cam;
  frame<float> the_frame;

  float cam_fov = 45.0f;
  float cam_near = 0.1f;
  float cam_far = 1000.0f;

  the_frame.set_perspective( radians( cam_fov ), ( float )screen.x / ( float )screen.y, cam_near, cam_far );

  cam.move_up( 1.75 );
  cam.move_forward( -2.265 );
  cam.move_right( -0.05 );

  glViewport( 0, 0, screen.x, screen.y );

  /*
   * Set up the scene
   */

  float move_amount = 5;
  float cam_rotation_amount = 5.0;

  scene s;

  //mesh::load_into_meshes( "../resources/guard/boblampclean.md5mesh", s, true );
  //mesh::load_into_meshes( "../resources/ak/ak.dae", s, true );
  mesh::load_into_meshes( "../resources/zombie/zombie.dae", s, true );

  /*
   * Set up the shaders
   */

  GLuint skinning_shader = 0;
  frm.load_shader( skinning_shader, GL_VERTEX_SHADER, "../shaders/gpu_skinning/gpu_skinning.vs" );
  frm.load_shader( skinning_shader, GL_FRAGMENT_SHADER, "../shaders/gpu_skinning/gpu_skinning.ps" );

  GLint skinning_mvp_mat_loc = glGetUniformLocation( skinning_shader, "mvp" );
  GLint skinning_bones_mat_loc = glGetUniformLocation( skinning_shader, "bones" );

  /*
   * Handle events
   */

  bool warped = false, ignore = true;
  vec2 movement_speed = vec2(0);

  auto event_handler = [&]( const sf::Event & ev )
  {
    switch( ev.type )
    {
      case sf::Event::MouseMoved:
        {
          vec2 mpos( ev.mouseMove.x / float( screen.x ), ev.mouseMove.y / float( screen.y ) );

          if( warped )
          {
            ignore = false;
          }
          else
          {
            frm.set_mouse_pos( ivec2( screen.x / 2.0f, screen.y / 2.0f ) );
            warped = true;
            ignore = true;
          }

          if( !ignore && all( notEqual( mpos, vec2( 0.5 ) ) ) )
          {
            cam.rotate( radians( -180.0f * ( mpos.x - 0.5f ) ), vec3( 0.0f, 1.0f, 0.0f ) );
            cam.rotate_x( radians( -180.0f * ( mpos.y - 0.5f ) ) );
            frm.set_mouse_pos( ivec2( screen.x / 2.0f, screen.y / 2.0f ) );
            warped = true;
          }
        }
      case sf::Event::KeyPressed:
        {
          if( ev.key.code == sf::Keyboard::Space )
          {
            cout << cam.pos << endl << cam.view_dir << endl << cam.up_vector;
          }
        }
      default:
        break;
    }
  };

  /*
   * Render
   */

  sf::Clock timer;
  timer.restart();

  sf::Clock movement_timer;
  movement_timer.restart();

  float orig_mov_amount = move_amount;

  glEnable( GL_FRAMEBUFFER_SRGB );

  frm.display( [&]
  {
    frm.handle_events( event_handler );

    float seconds = movement_timer.getElapsedTime().asMilliseconds() / 1000.0f;

    if( sf::Keyboard::isKeyPressed( sf::Keyboard::LShift ) || sf::Keyboard::isKeyPressed( sf::Keyboard::RShift ) )
    {
      move_amount = orig_mov_amount * 3.0f;
    }
    else
    {
      move_amount = orig_mov_amount;
    }

    if( seconds > 0.01667 )
    {
      //move camera
      if( sf::Keyboard::isKeyPressed( sf::Keyboard::A ) )
      {
        movement_speed.x -= move_amount;
      }

      if( sf::Keyboard::isKeyPressed( sf::Keyboard::D ) )
      {
        movement_speed.x += move_amount;
      }

      if( sf::Keyboard::isKeyPressed( sf::Keyboard::W ) )
      {
        movement_speed.y += move_amount;
      }

      if( sf::Keyboard::isKeyPressed( sf::Keyboard::S ) )
      {
        movement_speed.y -= move_amount;
      }

      cam.move_forward( movement_speed.y * seconds );
      cam.move_right( movement_speed.x * seconds );
      movement_speed *= 0.5;

      movement_timer.restart();
    }

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( skinning_shader );

    float time = timer.getElapsedTime().asMilliseconds() * 0.001f; 

    mat4 model = create_rotation( radians(-90), vec3(1, 0, 0 ) ); 
    mat4 view = cam.get_matrix();
    mat4 mvp = the_frame.projection_matrix * view * model;
    glUniformMatrix4fv( skinning_mvp_mat_loc, 1, false, &mvp[0][0] );

    mat4 bones[100];
    mesh::update_animation( time, s, bones );
    glUniformMatrix4fv( skinning_bones_mat_loc, 100, false, &bones[0][0][0] );

    for( int c = 0; c < s.meshes.size(); ++c )
    {
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, s.materials[c].diffuse_tex );

      s.meshes[c].render();
    }

    frm.get_opengl_error();
  }, silent );

  return 0;
}
