#!/usr/bin/env ruby

require 'fileutils'
require 'open-uri'
require 'rubygems'
require 'json'
require 'pp'
require 'xmlsimple'

$run_path = "/Users/bill/Documents/ITP/AIT-Creative Misuse"
$images_path = "#{$run_path}/stream"
$factual_api_key = "YOUR_KEY_HERE";
$output = []

def get_exif_data(filepath)
  
  exif_position = `/usr/bin/exiftool \"#{filepath}\" | grep \"GPS Position\"`
  exif_position = exif_position.strip
  
  if exif_position.length == 0
    return nil
  end
  
  tokens =  exif_position.split(':')[1].strip().split(",")
  #lat = 0; lng = 0;
  coords = {:lat => 0, :lng => 0}
  tokens.each_with_index do |toks, i|        
    
    coordToks = toks.strip().split(' ')
    hour = coordToks[0].to_i
    min = coordToks[2].gsub("'", "").to_f
    sec = coordToks[3].gsub("\"", "").to_f
    direction = coordToks[4].downcase
    fraction = (min * 60 + sec) / 3600.0
    
    coord = hour + fraction

    if (direction == 's' || direction == 'w')
      coord = coord * -1
    end
    
    if (i == 0)
      coords[:lat] = coord
    elsif (i == 1)
      coords[:lng] = coord
    end
    
  end
  
  return coords
  
end

def get_demographic_data(lat, lng)  
  begin
    url = URI::parse(URI::escape("http://api.v3.factual.com/geopulse/context?geo={\"$point\":[#{lat},#{lng}]}&KEY=#{$factual_api_key}"))
    response = open(url).string
    return JSON::parse(response)
  rescue Exception => e
    return nil
  end
end

def get_map_tile(lat, lng)
  url = URI::parse(URI::escape("http://maps.googleapis.com/maps/api/staticmap?center=#{lat},#{lng}&zoom=16&size=256x256&maptype=roadmap&sensor=false&markers=color:green|#{lat},#{lng}"))
  filename = "#{lat.abs}-#{lng.abs}.png"
  file_path = "/tmp/#{filename}"
  open(url) do |data|
    File.open(file_path, "wb") do |file|
      file.puts data.read
      return file_path
    end
  end
  return nil
end

def parse_demographic_data(demo_response)
  
  # pp demo_response  
  begin

    data = {}
    
    demo_data = demo_response["response"]["data"]["demographics"]
    $output << "demographics data:"
    $output << demo_data.inspect

    # age
    median_age = demo_data["age_and_sex"]["median_age"]
    data[:age_male] = median_age['male']
    data[:age_female] = median_age['female']

    # race
    race_data = demo_data["race_and_ethnicity"]["race"]
    origin_data = demo_data["race_and_ethnicity"]["origin"]
    data[:race_asian] = race_data['asian'].to_f
    data[:race_black] = race_data['black'].to_f
    latino = origin_data['hispanic_or_latino'].to_f
    white = race_data['white'].to_f
    data[:race_white] = white - latino
    data[:race_latino] = latino
    data[:race_other] = 100 - (data[:race_asian] + data[:race_black] + data[:race_white] + data[:race_latino]);
    
    # gender
    data[:gender_male] = demo_data["age_and_sex"]["male"]
    data[:gender_female] = demo_data["age_and_sex"]["female"]
    
    # income
    data[:income_level] = demo_data["income"]["median_income"]["score"]["national"]
    
    return data

  rescue Exception => e
    
    # ignore
    $output << "Error parsing demographics: #{e}"
    $output << e.backtrace
    return nil
    
  end
  
  return nil
  
end
  
begin
  
  image_data = []
  Dir.glob("#{$images_path}/*").each do |filename|

    $output << "Parsing #{filename}"    
    
    if filename.downcase.match(/\.(jpg|png)$/i)
      
      # we've got an image
      $output << "Found image #{filename}"
      if coords = get_exif_data(filename)
        $output << "Image located at #{coords.inspect}"
        if demo_response = get_demographic_data(coords[:lat], coords[:lng])          
          if data = parse_demographic_data(demo_response)
            data[:lat] = coords[:lat]
            data[:lng] = coords[:lng]
            pp data            
            #if map_path = get_map_tile(coords[:lat], coords[:lng])      
              # Store the image data for this photo
              # image_data.push({:data => data, :map_path => map_path})
              image_data.push({:data => data})
            #end
          else
            $output << "Could not parse demographic data for #{coords.inspect} in #{filename}"
          end          
        else
          $output << "Found no demographic data for #{coords.inspect} in #{filename}"
        end
      else
        $output << "Found no LatLng in EXIF: #{filename}"
      end
    end    
  end  

  # Write all the image data to a file 
  out_path = File.expand_path(File.join($run_path, "image_data.xml"))
  File.open(out_path, 'w+') do |f|
    f << XmlSimple.xml_out(image_data)
  end

  # Launch the Cinder app
  $output << "Running Cinder at #{Time.now}"
  runpath = File.join($run_path, "ImageMaker.app/Contents/MacOS/ImageMaker").gsub(" ", '\ ')
  cinder_result = system(runpath)
  $output << cinder_result

  # Export the files to iPhoto
  $output << "Running iPhoto Import at #{Time.now}:"
  runpath = File.join($run_path, "ImportOutput.app/Contents/MacOS/Application\ Stub").gsub(" ", '\ ')
  export_result = system(runpath)
  $output << export_result

  if export_result
  # Finally, remove the files we just processed
    $output << "Removing images at #{Time.now}:"
    $output << FileUtils.rm_rf(Dir.glob("#{$images_path}/*"))
  end
  
rescue Exception => e
  
  output_path = File.expand_path(File.join($run_path, "error_output.txt"))
  File.open(output_path, 'w+') do |f|
    f << "ERROR executing script at #{Time.now}:\n"
    f << e.inspect
    f << e.backtrace
  end
  
end

output_path = File.expand_path(File.join($run_path, "completed_output.txt"))
File.open(output_path, 'w+') do |f|
  f << "Completed at #{Time.now}\n"
  f << $output.join("\n")
end
